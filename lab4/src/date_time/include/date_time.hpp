
#include <string>
#include <ctime>

/**
 * @brief Класс для работы с датой и временем
 */
class DateTime {

public:

    /**
     * @brief Конструктор по умолчанию, инициализирующий текущую дату и время
     */
    DateTime();

    /**
     * @brief Конструктор, инициализирующий дату и время из time_t
     * 
     * @param timestamp Unix timestamp
     */
    DateTime(std::time_t timestamp);

    /**
     * @brief Конструктор, инициализирующий дату и время из строки
     * 
     * @param time_str Строка в формате "YYYY-MM-DD HH:MM:SS"
     */
    DateTime(const std::string& time_str);
    
    /**
     * @brief Преобразование в строку
     * 
     * @return Строка в формате "YYYY-MM-DD HH:MM:SS"
     */
    std::string toString() const;

    /**
     * @brief Получить Unix timestamp
     * 
     * @return Unix timestamp
     */
    std::time_t getTimestamp() const;

    /**
     * @brief Установить Unix timestamp
     * 
     * @param timestamp Unix timestamp
     */
    void setTimestamp(std::time_t timestamp);

    /**
     * @brief Оператор сравнения
     */
    bool operator<=(const DateTime& other) const;
    bool operator>=(const DateTime& other) const;
    bool operator<(const DateTime& other) const;
    bool operator>(const DateTime& other) const;
    bool operator==(const DateTime& other) const;

    /**
     * @brief Разница между датами в секундах
     */
    std::time_t operator-(const DateTime& other) const;

private:
    std::time_t timestamp;
};


