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
    m_root.uniqueId = "/";

    for (const auto& [path, entries] : cache) {
        FileInfo* parentDir = (path == "/") ? &m_root : findOrCreateDirectory(m_root, QString::fromStdString(path));

        for (const auto& entry : entries) {
            if (entry.m_name == "." || entry.m_name == ".." || entry.m_isSymLink) {
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
           
            if (parentDir->uniqueId == "/") {
                child.uniqueId = "/" + child.name;
            }
            else {
                child.uniqueId = parentDir->uniqueId + "/" + child.name;
            }

            
            parentDir->children.append(child);

        }
    }

    endResetModel();
}

void FileSystem::refreshDirectory(const std::string& path, const std::vector<DirectoryEntry>& entries) {
    // TODO: This deleting of '/' if it is the last character needs to be redesigned, it will become spaghetti code....
    QString qPath;
    if (path != "/") {
        QString qPath = QString::fromStdString(path.substr(0, path.size() - 1));
    }
    else {
        qPath = QString::fromStdString(path);
    }

    FileInfo* dirNode = findNode(qPath);
    if (!dirNode) {
        logger().error() << "Cannot find directory node for path: " << path;
        return;
    }
    logger().debug() << "\n=== Start Refresh ===";
    logger().debug() << "Path: " << path;
    logger().debug() << "Current number of children: " << dirNode->children.size();
    logger().debug() << "Number of new entries: " << entries.size();

    std::cout << "\n=== Start Refresh ===" << std::endl;
    std::cout << "Path: " << path << std::endl;
    std::cout << "Current number of children: " << dirNode->children.size() << std::endl;
    std::cout << "Number of new entries: " << entries.size() << std::endl;
    // Get the model index for this directory
    QModelIndex dirIndex;
    if (dirNode == &m_root) {
        dirIndex = QModelIndex();
    }
    else {
        int row = 0;
        if (dirNode->parent) {
            row = dirNode->parent->children.indexOf(*dirNode);
            if (row < 0) {
                logger().error() << "Invalid parent-child relationship detected";
                return;
            }
        }
        dirIndex = createIndex(row, 0, dirNode);
    }

    QVector<int> added;
    QVector<int> removed;
    QVector<int> modified;

    QHash<QString, int> existingChildren;
    for (int i = 0; i < dirNode->children.size(); ++i) {
        existingChildren[dirNode->children[i].name] = i;
    }

    QList<FileInfo> newChildren;
    for (const auto& entry : entries) {
        if (entry.m_name == "." || entry.m_name == ".." || entry.m_isSymLink) {
            continue;
        }

        QString entryName = QString::fromStdString(entry.m_name);
        FileInfo newInfo;
        newInfo.name = entryName;
        newInfo.isDirectory = entry.m_isDirectory;
        newInfo.size = entry.m_isDirectory ? "" : Commons::convertSize(entry.m_totalBytes);
        newInfo.tLastModifiedTime = entry.m_tLastModified;
        newInfo.permissions = QString::fromStdString(entry.m_permissions);
        newInfo.owner = QString::fromStdString(entry.m_owner);
        newInfo.parent = dirNode;
        newInfo.uniqueId = dirNode->uniqueId == "/" ?
            "/" + entryName : dirNode->uniqueId + "/" + entryName;

        auto existingIt = existingChildren.find(entryName);
        if (existingIt == existingChildren.end()) {
            added.append(newChildren.size());
        }
        else {
            int existingIndex = *existingIt;
            if (existingIndex >= 0 && existingIndex < dirNode->children.size()) {
                const FileInfo& existing = dirNode->children[existingIndex];
                if (existing.size != newInfo.size ||
                    existing.tLastModifiedTime != newInfo.tLastModifiedTime ||
                    existing.permissions != newInfo.permissions ||
                    existing.owner != newInfo.owner) {
                    modified.append(existingIndex);
                }
                // Preserve existing children for directories
                if (existing.isDirectory) {
                    newInfo.children = existing.children;
                }
            }
            existingChildren.remove(entryName);
        }
        newChildren.append(newInfo);
    }

   

    // Deleting remaining existingChildren which where not found
    for (auto it = existingChildren.begin(); it != existingChildren.end(); ++it) {
        if (*it >= 0 && *it < dirNode->children.size()) {
            removed.append(*it);
        }
    }

    logger().debug() << "Number of new children created: " << newChildren.size();
    logger().debug() << "Added items: " << added.size();
    logger().debug() << "Removed items: " << removed.size();
    logger().debug() << "Modified items: " << modified.size();

    std::cout << "Number of new children created: " << newChildren.size() << std::endl;
    std::cout << "Added items: " << added.size() << std::endl;
    std::cout << "Removed items: " << removed.size() << std::endl;
    std::cout << "Modified items: " << modified.size() << std::endl;


    if (added.isEmpty() && removed.isEmpty() && modified.isEmpty()) {
        logger().debug() << "No changes detected in directory: " << path;
        return;
    }

    // Fix parent pointers
    for (auto& child : newChildren) {
        child.parent = dirNode;
        if (child.isDirectory && !child.children.isEmpty()) {
            std::function<void(FileInfo&)> fixParents = [&fixParents](FileInfo& node) {
                for (auto& child : node.children) {
                    child.parent = &node;
                    if (child.isDirectory && !child.children.isEmpty()) {
                        fixParents(child);
                    }
                }
            };
            fixParents(child);
        }
    }

    if (!added.isEmpty() || !removed.isEmpty()) {
        logger().debug() << "About to assign new children";
        logger().debug() << "Old children count: " << dirNode->children.size();
        logger().debug() << "New children count: " << newChildren.size();

        std::cout << "About to assign new children" << std::endl;
        std::cout << "Old children count: " << dirNode->children.size() << std::endl;
        std::cout << "New children count: " << newChildren.size() << std::endl;
    }

    if (!removed.isEmpty()) {
        std::cout << "About to remove rows. Remove list: ";
        for (int idx : removed) {
            std::cout << idx << " ";
            std::cout << "Item at index: " << dirNode->children[idx].name.toStdString() << " ";
        }
        std::cout << std::endl;

        std::cout << "Current children before removal: " << std::endl;
        for (int i = 0; i < dirNode->children.size(); i++) {
            std::cout << i << ": " << dirNode->children[i].name.toStdString() << std::endl;
        }
    }

    if (!removed.isEmpty()) {
        std::sort(removed.begin(), removed.end());
        beginRemoveRows(dirIndex, removed.first(), removed.last());
        endRemoveRows();
    }

    if (!added.isEmpty()) {
        std::sort(added.begin(), added.end());
        beginInsertRows(dirIndex, added.first(), added.last());
    }

    if (removed.isEmpty() && added.isEmpty()) {
        for (int idx : modified) {
            if (idx >= 0 && idx < dirNode->children.size()) {
                auto it = std::find_if(newChildren.begin(), newChildren.end(),[&](const FileInfo& info) { 
                    return info.name == dirNode->children[idx].name; 
                });

                if (it != newChildren.end()) {
                    dirNode->children[idx].size = it->size;
                    dirNode->children[idx].tLastModifiedTime = it->tLastModifiedTime;
                    dirNode->children[idx].permissions = it->permissions;
                    dirNode->children[idx].owner = it->owner;
                    QModelIndex modifiedIndex = index(idx, 0, dirIndex);
                    emit dataChanged(modifiedIndex, index(idx, columnCount() - 1, dirIndex));
                }

            }
        }
    }
    else {
        dirNode->children = newChildren;
    }

    if (!added.isEmpty()) {
        endInsertRows();
    }

    if (!modified.isEmpty()) {
        for (int idx : modified) {
            if (idx >= 0 && idx < dirNode->children.size()) {
                QModelIndex modifiedIndex = index(idx, 0, dirIndex);
                emit dataChanged(modifiedIndex, index(idx, columnCount() - 1, dirIndex));
            }
        }
    }

    logger().debug() << "=== End Refresh ===\n";
    std::cout << "=== End Refresh ===\n" << std::endl;

  /*  for (const auto& child : dirNode->children) {
        std::cout << "Child: " << child.name.toStdString()
            << " Parent: " << static_cast<void*>(child.parent) << std::endl;

        logger().debug() << "Child: " << child.name.toStdString()
            << " Parent: " << static_cast<void*>(child.parent);
    }*/

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

void FileSystem::sort(int column, Qt::SortOrder order) {
    layoutAboutToBeChanged();

    auto comparator = [column, order](const FileInfo& a, const FileInfo& b) {
        int result = 0;

        switch (column) {
        case 0: // Name
            result = QString::compare(a.name, b.name, Qt::CaseInsensitive);
            break;
        case 1: // Size
            result = a.size.toULongLong() < b.size.toULongLong() ? -1 :
                (a.size.toULongLong() > b.size.toULongLong() ? 1 : 0);
            break;
        case 2: // Type
            result = a.isDirectory == b.isDirectory ? 0 : (a.isDirectory ? -1 : 1);
            break;
        case 3: // Date Modified
            result = a.tLastModifiedTime < b.tLastModifiedTime ? -1 :
                (a.tLastModifiedTime > b.tLastModifiedTime ? 1 : 0);
            break;
        case 4: // Permissions
            result = QString::compare(a.permissions, b.permissions, Qt::CaseInsensitive);
            break;
        case 5: // Owner
            result = QString::compare(a.owner, b.owner, Qt::CaseInsensitive);
            break;
        }

        return (order == Qt::AscendingOrder) ? result < 0 : result > 0;
    };

    std::function<void(FileInfo&)> sortChildren = [&comparator, &sortChildren](FileInfo& parent) {
        std::sort(parent.children.begin(), parent.children.end(), comparator);

        for (auto& child : parent.children) {
            child.parent = &parent;
            if (child.isDirectory) {
                sortChildren(child);
            }
        }
    };

    sortChildren(m_root);
    layoutChanged();
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
    if (!index.isValid()) {
       // logger().debug() << "parent(): Invalid index";
        std::cout << "parent(): Invalid index" << std::endl;
        return QModelIndex();
    }

    FileInfo* childItem = static_cast<FileInfo*>(index.internalPointer());
    if (!childItem || childItem == &m_root) {
        // logger().debug() << "parent(): Null child item or child is root";
        std::cout << "parent(): Null child item or child is root" << std::endl;
        return QModelIndex();
    }

    //    logger().debug() << "parent(): Processing child: " << childItem->name.toStdString()
      //  << " uniqueId: " << childItem->uniqueId.toStdString();
    //std::cout << "parent(): Processing child: " << childItem->name.toStdString()
     //   << " uniqueId: " << childItem->uniqueId.toStdString() << std::endl;

    FileInfo* parentItem = childItem->parent;
    if (!parentItem) {
        //  logger().debug() << "parent(): Child has null parent: " << childItem->name.toStdString();
        std::cout << "parent(): Child has null parent: " << childItem->name.toStdString() << std::endl;
        return QModelIndex();
    }

    //    logger().debug() << "parent(): Parent is: " << parentItem->name.toStdString()
    //    << " uniqueId: " << parentItem->uniqueId.toStdString();
   // std::cout << "parent(): Parent is: " << parentItem->name.toStdString()
     //   << " uniqueId: " << parentItem->uniqueId.toStdString() << std::endl;

    // If parent is root, handle specially
    if (parentItem == &m_root || parentItem->uniqueId == "/") {
        std::cout << "parent(): Parent is root, searching for child position" << std::endl;
        for (int i = 0; i < m_root.children.size(); ++i) {
            if (m_root.children[i].uniqueId == childItem->uniqueId) {
                std::cout << "parent(): Found child at position: " << i << std::endl;
                return createIndex(i, 0, parentItem);
            }
        }
        // logger().error() << "parent(): Child not found in root's children";
        std::cout << "parent(): Child not found in root's children" << std::endl;
        return QModelIndex();
    }

    // For non-root parents
    FileInfo* grandParentItem = parentItem->parent;
    if (!grandParentItem) {
        // logger().error() << "parent(): Non-root parent has no grandparent";
        std::cout << "parent(): Non-root parent has no grandparent" << std::endl;
        return QModelIndex();
    }

    // Find parent's position in grandparent's children
    for (int i = 0; i < grandParentItem->children.size(); ++i) {
        if (grandParentItem->children[i].uniqueId == parentItem->uniqueId) {
            //  logger().debug() << "parent(): Found parent at position: " << i;
            std::cout << "parent(): Found parent at position: " << i << std::endl;
            return createIndex(i, 0, parentItem);
        }
    }

    //logger().error() << "parent(): Parent not found in grandparent's children";
    std::cout << "parent(): Parent not found in grandparent's children" << std::endl;
    return QModelIndex();
}

int FileSystem::rowCount(const QModelIndex& parent) const {
    FileInfo* parentItem = parent.isValid() ? getItem(parent) : const_cast<FileInfo*>(&m_root);
    return parentItem ? parentItem->children.size() : 0;
}

int FileSystem::columnCount(const QModelIndex&) const {
    return 6; // Columns: Name, Size, Type, Date Modified, Permissions, Owner
}

QVariant FileSystem::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        logger().debug() << "Invalid index in data()";
        std::cout << "Invalid index in data()" << std::endl;
        return QVariant();
    }
    
    FileInfo* item = getItem(index);
    if (!item) {
        logger().debug() << "Null item in data() for index row:" << index.row()
            << " column:" << index.column();
        std::cout << "Null item in data() for index row:" << index.row()
            << " column:" << index.column() << std::endl;
        return QVariant();
    }

    if (!item->parent) {
        logger().debug() << "Item has null parent: " << item->name.toStdString();
        std::cout << "Item has null parent: " << item->name.toStdString() << std::endl;
    }

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

FileInfo* FileSystem::findOrCreateDirectory(FileInfo& root, const QString& path) {
    QStringList pathParts = path.split("/", Qt::SkipEmptyParts);
    FileInfo* current = &root;
    QString currentPath = current->uniqueId;

    for (const auto& part : pathParts) {
        if (current->uniqueId == "/") {
            currentPath = "/" + part;

        }
        else {
            currentPath = current->uniqueId + "/" + part;
        }

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
    }
    return current;
}

void FileSystem::printUniqueIdsRecursively(const FileInfo& node, int depth) const {
    std::string indent(depth * 2, ' ');
   // logger().info() << indent << "Unique ID: " << node.uniqueId.toStdString();

    for (const auto& child : node.children) {
        printUniqueIdsRecursively(child, depth + 1);
    }
}

FileInfo* FileSystem::findNode(const QString& uniqueId) {
    if (uniqueId == "/") {
        return &m_root;
    }

    std::function<FileInfo* (FileInfo*, const QString&)> findRecursive =
        [&findRecursive](FileInfo* current, const QString& id) -> FileInfo* {
        if (current->uniqueId == id) {
            return current;
        }

        for (auto& child : current->children) {
            if (FileInfo* found = findRecursive(&child, id)) {
                return found;
            }
        }
        return nullptr;
    };

    return findRecursive(&m_root, uniqueId);
}


void FileSystem::printUniqueIds() const {
    printUniqueIdsRecursively(m_root, 0);
}