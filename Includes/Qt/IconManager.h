#ifndef SFTP_CLIENT_ICONMANAGER_H
#define SFTP_CLIENT_ICONMANAGER_H
#include <QIcon>

class IconManager {
public:
    static QIcon getStaticDirectoryIcon() {
        static QIcon directoryIcon(":/icons/dir.png");
        return directoryIcon;
    }

    static QIcon getStaticFileIcon() {
        static QIcon fileIcon(":/icons/file.png");
        return fileIcon;
    }
};
#endif // SFTP_CLIENT_ICONMANAGER_H