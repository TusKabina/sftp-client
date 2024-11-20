#ifndef SFTP_CLIENT_LOGGER_H
#define SFTP_CLIENT_LOGGER_H

#include <QObject>
#include <QTextEdit>
#include <QMutex>
#include <QMap>
#include <sstream>

enum LogLevel {
	Debug,
	Info,
	Warning,
	Error,
	Critical
};

class Logger : public QObject {
    Q_OBJECT

public:
    static Logger& instance();
    static void log(const QString& message, LogLevel level = Info);
    void setLogWidget(QTextEdit* textEdit);
    void setLogFormat(LogLevel level, const QTextCharFormat& format);

    class LogStream {
    public:
        LogStream(Logger& logger, LogLevel level);

        LogStream(const LogStream&) = delete;
        LogStream& operator=(const LogStream&) = delete;
        
        LogStream(LogStream&& other) noexcept;
        LogStream& operator=(LogStream&& other) noexcept;


        template <typename T>
        LogStream& operator<<(const T& value) {
            m_stream << value;
            return *this;
        }

        LogStream& operator<<(std::ostream& (*manip)(std::ostream&));

        void flush();

        ~LogStream();

    private:
        Logger& m_logger;
        LogLevel m_level;
        std::stringstream m_stream;
        bool m_flushed = false;
    };

    LogStream debug();
    LogStream info();
    LogStream warning();
    LogStream error();
    LogStream critical();

signals:
    void appendMessage(const QString& message, LogLevel level);

private slots:
    void handleAppendMessage(const QString& message, LogLevel level);

private:
    QTextEdit* m_textEdit;
    QMutex m_mutex;
    QMap<LogLevel, QTextCharFormat> m_formatMap;
   
    explicit Logger(QObject* parent = nullptr);
    //Logger(const Logger&) = delete;
    //Logger& operator=(const Logger&) = delete;
};

Logger& logger();

#endif //SFTP_CLIENT_LOGGER_H