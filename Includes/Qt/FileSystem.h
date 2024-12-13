#ifndef SFTP_CLIENT_FILESYSTEM_H
#define SFTP_CLIENT_FILESYSTEM_H

#include <QAbstractItemModel>
#include <QString>
#include <QList>
#include <qtreeview.h>
#include "DirectoryEntry.h"
#include "Utilities/Commons.h"

struct FileInfo {
    QString name;
    bool isDirectory;
    bool created = false;
    QString size;
    QString lastModified; // TODO: delete
    time_t tLastModifiedTime;
    QString permissions;
    QString owner;
    QList<FileInfo> children;
    FileInfo* parent = nullptr;
    QString uniqueId;

    bool operator==(const FileInfo& other) const {
        return uniqueId == other.uniqueId;
    }
};

class FileSystem : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit FileSystem(QObject* parent = nullptr);
    void setRoot(const FileInfo& rootData);
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void assignParentPointers(FileInfo* parent);
    void populateFileSystem(const std::map<std::string, std::vector<DirectoryEntry>>& cache);

   void printUniqueIds() const;

private:
    FileInfo m_root;
    FileInfo* getItem(const QModelIndex& index) const;
    FileInfo* findOrCreateDirectory(FileInfo& root, const QString& path);
    void printUniqueIdsRecursively(const FileInfo& node, int depth) const;
};

#endif // SFTP_CLIENT_FILESYSTEM_H