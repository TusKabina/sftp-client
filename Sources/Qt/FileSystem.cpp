#include "Qt/FileSystem.h"
#include "Qt/IconManager.h"
#include "Utilities/Logger.h"

FileSystem::FileSystem(QObject* parent)
    : QAbstractItemModel(parent) {}

void FileSystem::assignParentPointers(FileInfo* parent) {
    for (auto& child : parent->children) {
        child.parent = parent;
        assignParentPointers(&child);
    }
}

void FileSystem::populateFileSystem(const std::map<std::string, std::vector<DirectoryEntry>>& cache) {
    beginResetModel();
    m_root = FileInfo();
    m_root.name = "/";
    m_root.isDirectory = true;

    for (const auto& [path, entries] : cache) {
        FileInfo* parentDir = (path == "/") ? &m_root : findOrCreateDirectory(m_root, QString::fromStdString(path));

        for (const auto& entry : entries) {
            if (entry.m_name == "." || entry.m_name == "..") {
                continue;
            }

            FileInfo child;
            child.name = QString::fromStdString(entry.m_name);
            child.isDirectory = entry.m_isDirectory;
            child.size = entry.m_isDirectory ? "" : Commons::convertSize(entry.m_totalBytes);
            child.tLastModifiedTime = entry.m_tLastModified;
            child.permissions = QString::fromStdString(entry.m_permissions);
            child.owner = QString::fromStdString(entry.m_owner);
            child.parent = parentDir;
            child.uniqueId = parentDir->uniqueId + "/" + child.name;

            parentDir->children.append(child);
        }
    }
    endResetModel();
}

void FileSystem::setRoot(const FileInfo& rootData) {
    beginResetModel();
    m_root = rootData;

    for (auto& child : m_root.children) {
        child.parent = &m_root;
        assignParentPointers(&child);
    }

    endResetModel();
}

//void FileSystem::sort(int column, Qt::SortOrder order) {
//    layoutAboutToBeChanged();
//    emit beginRefreshModel();
//    auto comparator = [column, order](const FileInfo& a, const FileInfo& b) {
//        int result = 0;
//
//        switch (column) {
//        case 0: // Name
//            result = QString::compare(a.name, b.name, Qt::CaseInsensitive);
//            break;
//        case 1: // Size
//            result = a.size.toULongLong() < b.size.toULongLong() ? -1 :
//                (a.size.toULongLong() > b.size.toULongLong() ? 1 : 0);
//            break;
//        case 2: // Type
//            result = a.isDirectory == b.isDirectory ? 0 : (a.isDirectory ? -1 : 1);
//            break;
//        case 3: // Date Modified
//            result = a.tLastModifiedTime < b.tLastModifiedTime ? -1 :
//                (a.tLastModifiedTime > b.tLastModifiedTime ? 1 : 0);
//            break;
//        case 4: // Permissions
//            result = QString::compare(a.permissions, b.permissions, Qt::CaseInsensitive);
//            break;
//        case 5: // Owner
//            result = QString::compare(a.owner, b.owner, Qt::CaseInsensitive);
//            break;
//        }
//
//        return (order == Qt::AscendingOrder) ? result < 0 : result > 0;
//    };
//
//    std::function<void(FileInfo&)> sortChildren = [&comparator, &sortChildren](FileInfo& parent) {
//        std::sort(parent.children.begin(), parent.children.end(), comparator);
//
//        for (auto& child : parent.children) {
//            child.parent = &parent;
//            if (child.isDirectory) {
//                sortChildren(child);
//            }
//        }
//    };
//
//    sortChildren(m_root);
//    layoutChanged();
//    emit endRefreshModel();
//}

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
    if (!index.isValid()) {
        return QModelIndex();
    }

    FileInfo* childItem = static_cast<FileInfo*>(index.internalPointer());
    if (!childItem || childItem == &m_root) {
        return QModelIndex(); // Root has no parent
    }

    FileInfo* parentItem = childItem->parent;
    if (!parentItem) {
        return QModelIndex(); // top-level item
    }

    FileInfo* grandParentItem = parentItem->parent;
    if (!grandParentItem) {
        int row = m_root.children.indexOf(*parentItem);
        return createIndex(row, 0, parentItem);
    }

    auto it = std::find(grandParentItem->children.begin(), grandParentItem->children.end(), *parentItem);
    if (it == grandParentItem->children.end()) {
        logger().error() << "Parent item not found in grandparent's children list.";
        return QModelIndex();
    }

    int row = std::distance(grandParentItem->children.begin(), it);
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
    if (!index.isValid()) return QVariant();

    FileInfo* item = getItem(index);
    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0: return item->name;
        case 1: return item->size;
        case 2: return item->isDirectory ? "Folder" : "File";
        case 3:
            return QDateTime::fromTime_t(item->tLastModifiedTime).toString("MM/dd/yyyy HH:mm:ss");
        case 4: return item->permissions;
        case 5: return item->owner;
        }
        break;

    case Qt::DecorationRole:
        if (index.column() == 0) {
            return item->isDirectory ? IconManager::getStaticDirectoryIcon() : IconManager::getStaticFileIcon();
        }
        break;
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

FileInfo* FileSystem::findOrCreateDirectory(FileInfo& root, const QString& path)
{
    QStringList pathParts = path.split("/", Qt::SkipEmptyParts);
    FileInfo* current = &root;
    QString currentPath = current->uniqueId;

    for (const auto& part : pathParts) {
        currentPath += "/" + part;

        auto it = std::find_if(current->children.begin(), current->children.end(),
            [&part](const FileInfo& child) { return child.name == part && child.isDirectory; });

        if (it != current->children.end()) {
            current = &(*it);
        }
        else {
            FileInfo newDir;
            newDir.name = part;
            newDir.isDirectory = true;
            newDir.parent = current;
            newDir.uniqueId = currentPath;

            current->children.append(newDir);
            current = &current->children.last();
        }
        return current;
    }
}
