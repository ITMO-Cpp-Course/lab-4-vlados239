#include <catch2/catch_all.hpp>

#include <cstdio>
#include <filesystem>
#include <memory>

#include "resource_core/resource_core.hpp"

namespace fs = std::filesystem;
using namespace lab4::resource;

// Вспомогательная функция: создаёт временный файл с заданным содержимым и возвращает путь к нему.
static std::string makeTempFile(const std::string& content = "")
{
    auto tmp = fs::temp_directory_path() / "lab4_test_";
    static int counter = 0;
    std::string path = tmp.string() + std::to_string(++counter);

    std::FILE* f = std::fopen(path.c_str(), "w");
    if (!f)
        throw std::runtime_error("Не удалось создать временный файл");
    if (!content.empty())
        std::fwrite(content.data(), 1, content.size(), f);
    std::fclose(f);
    return path;
}

// ─── ResourceError ───────────────────────────────────────────────────────────

TEST_CASE("Открытие несуществующего файла бросает ResourceError", "[ResourceError]")
{
    REQUIRE_THROWS_AS(FileHandle("/no/such/file/here.txt"), ResourceError);
}

TEST_CASE("Сообщение ResourceError содержит путь к файлу", "[ResourceError]")
{
    const std::string path = "/no/such/file/here.txt";
    try
    {
        FileHandle fh(path);
        FAIL("Ожидалось исключение ResourceError");
    }
    catch (const ResourceError& e)
    {
        REQUIRE(std::string(e.what()).find(path) != std::string::npos);
    }
}

// ─── FileHandle – базовый RAII ───────────────────────────────────────────────

TEST_CASE("FileHandle успешно открывает существующий файл", "[FileHandle]")
{
    std::string path = makeTempFile("hello");
    FileHandle fh(path);
    REQUIRE(fh.isOpen());
    REQUIRE(fh.path() == path);
    fs::remove(path);
}

TEST_CASE("FileHandle автоматически закрывает файл при уничтожении объекта", "[FileHandle]")
{
    std::string path = makeTempFile();
    {
        FileHandle fh(path);
        REQUIRE(fh.isOpen());
    } // fh уничтожается здесь — деструктор вызывает close()
    REQUIRE(fs::remove(path)); // файл должен успешно удалиться — дескриптор освобождён
}

TEST_CASE("После вызова close() метод isOpen() возвращает false", "[FileHandle]")
{
    std::string path = makeTempFile();
    FileHandle fh(path);
    fh.close();
    REQUIRE_FALSE(fh.isOpen());
    fs::remove(path);
}

TEST_CASE("FileHandle корректно оборачивает уже открытый FILE*", "[FileHandle]")
{
    std::string path = makeTempFile();
    std::FILE* raw = std::fopen(path.c_str(), "r");
    REQUIRE(raw != nullptr);

    FileHandle fh(raw, path); // передаём уже открытый дескриптор
    REQUIRE(fh.isOpen());
    REQUIRE(fh.get() == raw);
    REQUIRE(fh.path() == path);
    // fh закроет raw в деструкторе — вручную fclose не нужен
    fs::remove(path);
}

TEST_CASE("Конструктор от нулевого FILE* бросает ResourceError", "[FileHandle]")
{
    REQUIRE_THROWS_AS(FileHandle(nullptr, ""), ResourceError);
}

// ─── FileHandle – передача владения ─────────────────────────────────────────

TEST_CASE("Move-конструктор передаёт владение файлом новому объекту", "[FileHandle]")
{
    std::string path = makeTempFile("data");
    FileHandle a(path);
    FileHandle b(std::move(a));

    REQUIRE_FALSE(a.isOpen()); // a больше не владеет ресурсом
    REQUIRE(b.isOpen());       // b теперь владелец
    REQUIRE(b.path() == path);
    fs::remove(path);
}

TEST_CASE("Move-присваивание закрывает старый ресурс и принимает новый", "[FileHandle]")
{
    std::string p1 = makeTempFile("first");
    std::string p2 = makeTempFile("second");

    FileHandle a(p1);
    FileHandle b(p2); // b владеет p2
    b = std::move(a); // b должен закрыть p2 и принять p1

    REQUIRE_FALSE(a.isOpen()); // a отдал владение
    REQUIRE(b.isOpen());       // b владеет p1
    REQUIRE(b.path() == p1);
    fs::remove(p1);
    fs::remove(p2);
}

// ─── file_io – чтение и запись ───────────────────────────────────────────────

TEST_CASE("writeToFile и readFromFile корректно записывают и читают данные", "[file_io]")
{
    std::string path = makeTempFile();

    {
        FileHandle wh(path, "w");
        writeToFile(wh, "Hello, RAII!"); // запись через свободную функцию
    } // файл закрывается и данные сбрасываются на диск
    {
        FileHandle rh(path, "r");
        REQUIRE(readFromFile(rh) == "Hello, RAII!"); // чтение через свободную функцию
    }

    fs::remove(path);
}

TEST_CASE("writeToFile бросает ResourceError при записи в закрытый файл", "[file_io]")
{
    std::string path = makeTempFile();
    FileHandle fh(path, "w");
    fh.close();
    REQUIRE_THROWS_AS(writeToFile(fh, "oops"), ResourceError);
    fs::remove(path);
}

TEST_CASE("readFromFile бросает ResourceError при чтении из закрытого файла", "[file_io]")
{
    std::string path = makeTempFile("данные");
    FileHandle fh(path, "r");
    fh.close();
    REQUIRE_THROWS_AS(readFromFile(fh), ResourceError);
    fs::remove(path);
}

// ─── ResourceManager – кеш ───────────────────────────────────────────────────

TEST_CASE("Повторное открытие того же пути возвращает один и тот же объект из кеша", "[ResourceManager]")
{
    std::string path = makeTempFile("cached");
    ResourceManager mgr;

    auto h1 = mgr.open(path);
    auto h2 = mgr.open(path);

    REQUIRE(h1.get() == h2.get()); // одинаковый адрес — один объект в памяти
    fs::remove(path);
}

TEST_CASE("contains возвращает true пока ресурс жив и false после его уничтожения", "[ResourceManager]")
{
    std::string path = makeTempFile();
    ResourceManager mgr;

    {
        auto h = mgr.open(path);
        REQUIRE(mgr.contains(path)); // ресурс жив — есть в кеше
    } // h уничтожен — weak_ptr истёк

    REQUIRE_FALSE(mgr.contains(path)); // ресурс мёртв — contains возвращает false
    fs::remove(path);
}

TEST_CASE("contains возвращает false для пути которого нет в кеше", "[ResourceManager]")
{
    ResourceManager mgr;
    REQUIRE_FALSE(mgr.contains("/несуществующий/путь.txt"));
}

TEST_CASE("После освобождения всех ссылок повторное открытие создаёт новый объект", "[ResourceManager]")
{
    std::string path = makeTempFile("expired");
    ResourceManager mgr;

    FileHandle* raw = nullptr;
    {
        auto h1 = mgr.open(path);
        raw = h1.get(); // запоминаем адрес
    } // h1 уничтожен — счётчик shared_ptr = 0 — weak_ptr истёк

    auto h2 = mgr.open(path);
    REQUIRE(h2.get() != raw); // новый объект по другому адресу
    fs::remove(path);
}

TEST_CASE("Кеш увеличивается при открытии разных файлов", "[ResourceManager]")
{
    std::string p1 = makeTempFile();
    std::string p2 = makeTempFile();
    ResourceManager mgr;

    auto h1 = mgr.open(p1);
    auto h2 = mgr.open(p2);

    REQUIRE(mgr.cacheSize() == 2);
    fs::remove(p1);
    fs::remove(p2);
}

TEST_CASE("evictExpired удаляет из кеша записи с истёкшими weak_ptr", "[ResourceManager]")
{
    std::string path = makeTempFile();
    ResourceManager mgr;

    {
        auto h = mgr.open(path);
    } // h сразу уничтожается — weak_ptr истекает

    REQUIRE(mgr.cacheSize() == 1); // мёртвая запись всё ещё в кеше
    mgr.evictExpired();
    REQUIRE(mgr.cacheSize() == 0); // после очистки кеш пуст
    fs::remove(path);
}

// ─── Совместное владение ─────────────────────────────────────────────────────

TEST_CASE("Ресурс остаётся живым пока хотя бы один shared_ptr на него существует", "[ResourceManager]")
{
    std::string path = makeTempFile("shared");
    ResourceManager mgr;

    std::shared_ptr<FileHandle> h2;
    {
        auto h1 = mgr.open(path); // счётчик = 1
        h2 = h1;                  // второй владелец, счётчик = 2
    } // h1 уничтожен, счётчик = 1 — объект ещё жив

    REQUIRE(h2->isOpen()); // файл до сих пор открыт
    fs::remove(path);
}
