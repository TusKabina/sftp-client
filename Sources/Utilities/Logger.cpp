#include "Utilities/Logger.h"
#include <QDateTime>
#include <QThread>
#include <iostream>
#include <sstream>

Logger& logger() {
    return Logger::instance();
}

Logger::LogStream::LogStream(Logger& logger, LogLevel level) : m_logger(logger), m_level(level) {}

Logger::LogStream::LogStream(Logger::LogStream&& other) noexcept : m_logger(other.m_logger), m_level(other.m_level), m_stream(std::move(other.m_stream)) {}

Logger::LogStream& Logger::LogStream::operator=(LogStream&& other) noexcept {
    if (this != &other) {
        m_stream = std::move(other.m_stream);
        m_level = other.m_level;
    }
    return *this;
}

Logger::LogStream::~LogStream() {}

Logger::LogStream Logger::debug() {
    return Logger::LogStream(*this, LogLevel::Debug);
}

Logger::LogStream Logger::info() {
    return Logger::LogStream(*this, LogLevel::Info);
}

Logger::LogStream Logger::warning() {
    return Logger::LogStream(*this, LogLevel::Warning);
}

Logger::LogStream Logger::error() {
    return Logger::LogStream(*this, LogLevel::Error);
}

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::Logger(QObject* parent)
    : QObject(parent), m_textEdit(nullptr) {
    QTextCharFormat format;

    // Debug
    format.setForeground(Qt::gray);
    m_formatMap[Debug] = format;

    // Info
    format.setForeground(Qt::black);
    m_formatMap[Info] = format;

    // Warning
    format.setForeground(QColor("orange"));
    m_formatMap[Warning] = format;

    // Error
    format.setForeground(Qt::red);
    format.setFontWeight(QFont::Bold);
    m_formatMap[Error] = format;

    // Critical
    format.setForeground(Qt::white);
    format.setBackground(Qt::red);
    format.setFontWeight(QFont::Bold);
    m_formatMap[Critical] = format;

    connect(this, &Logger::appendMessage, this, &Logger::handleAppendMessage);
}

void Logger::setLogWidget(QTextEdit* textEdit) {
    QMutexLocker locker(&m_mutex);
    m_textEdit = textEdit;
}

void Logger::setLogFormat(LogLevel level, const QTextCharFormat& format) {
    QMutexLocker locker(&m_mutex);
    m_formatMap[level] = format;
}

void Logger::log(const QString& message, LogLevel level) {
    Logger& logger = Logger::instance();

    QString timeStamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString formattedMessage = QString("[%1] %2").arg(timeStamp, message);

    emit logger.appendMessage(formattedMessage, level);
}

void Logger::handleAppendMessage(const QString& message, LogLevel level) {
    QMutexLocker locker(&m_mutex);

    if (!m_textEdit) {
        return;
    }

    if (QThread::currentThread() != m_textEdit->thread()) {
        QMetaObject::invokeMethod(this, "handleAppendMessage", Qt::QueuedConnection,
            Q_ARG(QString, message), Q_ARG(LogLevel, level));
        return;
    }

    QTextCursor cursor(m_textEdit->document());
    cursor.movePosition(QTextCursor::End);

    QTextCharFormat format = m_formatMap.value(level, QTextCharFormat());

    cursor.insertText(message + "\n", format);

    m_textEdit->setTextCursor(cursor);
    m_textEdit->ensureCursorVisible();
}