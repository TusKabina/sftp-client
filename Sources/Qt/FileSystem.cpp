#include "Qt/FileSystem.h"

FileSystem::FileSystem(QObject* parent)
    : QAbstractItemModel(parent) {}

void FileSystem::setRoot(const FileInfo& rootData) {
    beginResetModel();
    m_root = rootData;
    endResetModel();
}

QModelIndex FileSystem::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    FileInfo* parentItem = parent.isValid() ? getItem(parent) : const_cast<FileInfo*>(&m_root);
    if (parentItem && row >= 0 && row < parentItem->children.size()) {
        return createIndex(row, column, &parentItem->children[row]);
    }
    return QModelIndex();
}

QModelIndex FileSystem::parent(const QModelIndex& index) const {
    if (!index.isValid()) return QModelIndex();

    FileInfo* childItem = static_cast<FileInfo*>(index.internalPointer());
    if (!childItem || childItem == &m_root) return QModelIndex();

    FileInfo* parentItem = childItem->parent;
    if (!parentItem) return QModelIndex();

    FileInfo* grandParentItem = parentItem->parent;
    int row = grandParentItem ? grandParentItem->children.indexOf(*parentItem) : 0;
    return createIndex(row, 0, parentItem);
}

int FileSystem::rowCount(const QModelIndex& parent) const {
    FileInfo* parentItem = parent.isValid() ? getItem(parent) : const_cast<FileInfo*>(&m_root);
    return parentItem ? parentItem->children.size() : 0;
}

int FileSystem::columnCount(const QModelIndex&) const {
    return 6; // Columns: Name, Size, Type, Date Modified, Permissions, Owner
}

QVariant FileSystem::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole) return QVariant();

    FileInfo* item = getItem(index);
    switch (index.column()) {
        case 0: return item->name;
        case 1: return item->size;
        case 2: return item->isDirectory ? "Folder" : "File";
        case 3: return item->lastModified;
        case 4: return item->permissions;
        case 5: return item->owner;
    }
    return QVariant();
}

QVariant FileSystem::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return "Name";
            case 1: return "Size";
            case 2: return "Type";
            case 3: return "Date Modified";
            case 4: return "Permissions";
            case 5: return "Owner";
        }
    }
    return QVariant();
}

FileInfo* FileSystem::getItem(const QModelIndex& index) const {
    return index.isValid() ? static_cast<FileInfo*>(index.internalPointer()) : nullptr;
}