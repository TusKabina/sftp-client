#ifndef SFTP_CLIENT_FILESYSTEM_H
#define SFTP_CLIENT_FILESYSTEM_H

#include <QAbstractItemModel>
#include <QString>
#include <QList>

struct FileInfo {
    QString name;
    bool isDirectory;
    QString size;
    QString lastModified;
    QString permissions;
    QString owner;
    QList<FileInfo> children;
    FileInfo* parent = nullptr;

    bool operator==(const FileInfo& other) const {
        return name == other.name && isDirectory == other.isDirectory;
    }
};

class FileSystem : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit FileSystem(QObject* parent = nullptr);
    void setRoot(const FileInfo& rootData);

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    FileInfo m_root;
    FileInfo* getItem(const QModelIndex& index) const;
};

#endif // SFTP_CLIENT_FILESYSTEM_H