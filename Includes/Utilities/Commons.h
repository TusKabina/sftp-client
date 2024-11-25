#ifndef SFTP_CLIENT_COMMONS_H
#define SFTP_CLIENT_COMMONS_H

#include <string>
#include <QDateTime>
#include <QString>

namespace Commons {
    inline std::string GetDirectoryName(const std::string& name) {
        size_t pos = name.find_last_of("\\/");
        return (std::string::npos == pos) ? "" : name.substr(0, pos);
    }

    inline std::string FileName(const std::string& path) {
        return path.substr(path.find_last_of("/\\") + 1);
    }

    inline QString convertSize(qint64 size) {
        QStringList units = { "B", "KB", "MB", "GB", "TB" };
        int unitIndex = 0;
        double sizeInUnits = size;

        while (sizeInUnits > 1024.0 && unitIndex < units.size() - 1) {
            sizeInUnits /= 1024.0;
            unitIndex++;
        }

        return QString::number(sizeInUnits, 'f', 2) + " " + units[unitIndex];
    }

    inline QString formatDateModified(const QString& rawDate) {
        QDateTime dateTime = QDateTime::fromString(rawDate, "MMM dd yyyy");
        if (!dateTime.isValid()) {
            dateTime = QDateTime::fromString(rawDate, "MMM dd hh:mm");
        }
        if (dateTime.isValid()) {
            return dateTime.toString("dd/MM/yyyy hh:mm");
        }
        return rawDate;
    }
}
#endif //SFTP_CLIENT_COMMONS_H