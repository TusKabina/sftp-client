#ifndef SFTP_CLIENT_MAINGUI_H
#define SFTP_CLIENT_MAINGUI_H
#include <QMenu>
#include <QWidget>
#include <QTreeView>
#include <QTreeWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QModelIndex>
#include <QMouseEvent>
#include <QTreeWidgetItem>
#include <QString>
#include <QVBoxLayout>
#include <QtWidgets/qfilesystemmodel.h>
#include "ThreadPool.h"
#include "TransferManager.h"
#include <qdrag.h>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QMimeData>
#include <QIcon>
#include <qcombobox.h>
#include <QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include "Utilities/Commons.h"

class TreeView : public QTreeView {
	Q_OBJECT
signals:
	void RightClickAction(QMouseEvent* event);
public:
	TreeView(QWidget* parent = nullptr) : QTreeView(parent) {};
	void mousePressEvent(QMouseEvent* event) override;
	void startDrag(Qt::DropActions supportedActions) override;
	void dropEvent(QDropEvent* event) override;
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dragMoveEvent(QDragMoveEvent* event) override;
};

class TreeWidget : public QTreeWidget {
	Q_OBJECT
signals:
	void RightClickAction(QMouseEvent* event);
public:
	TreeWidget(QWidget* parent = nullptr);
	void mousePressEvent(QMouseEvent* event) override;
	void startDrag(Qt::DropActions supportedActions) override;
	void dropEvent(QDropEvent* event) override;
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dragMoveEvent(QDragMoveEvent* event) override;
};

class TreeViewWidget : public QWidget {
	Q_OBJECT
signals:
public slots:
	void onClickedTreeView(const QModelIndex& index);
	void onConnectButtonClicked();
	void onRightClickedAction(QMouseEvent* event);
	void onRightClickedActionTreeWidget(QMouseEvent* event);
	void onRightClickedActionTransferStatusWidget(QMouseEvent* event);
	void processTreeWidgetItemClicked(QTreeWidgetItem* item, int index);
	void eventFromThreadPoolReceived(int);
	void onDirectoryCacheUpdated(const std::string& path);
	void onRemoteFolderKeyPressed();
	void onTransferStatusUpdated(const TransferStatus& transferStatus);
	void onCopyAction();
	void onCutAction();
	void onPasteAction();
	void onDownloadAction();
	void onDeleteRemoteAction();
	void onuploadAction();
	void onDeleteLocalAction();
	void onCancelAction();
	void onRemoveAction();
	void onPauseAction();
	void onResumeAction();
	void onRenameLocalAction();
	void onCopyLocalAction();
	void onCutLocalAction();
	void onPasteLocalAction();
	void onLogLevelChanged(int index);
public:
	enum class TransferStatusHeader {
		FILE_NAME = 0,
		TRANSFER_STATE,
		SOURCE,
		DESTINATION,
		BYTES_TRANSFERRED,
		SPEED,
		PROGRESS
	};
	TreeViewWidget();
	void populateTreeView();
	void refreshTreeViewRoot(const std::string& path);
	void updateTreeView(const std::string& path);
	void findAndExpandPath(const QString& path);
	void populateTreeWidgetViewDirectory(QTreeWidgetItem* parentItem, const QString& path);
	bool connectToRemote();
	void disconnectFromRemote();

	QTextEdit& getDebugLog() { return m_textDebugLog; }
	TransferManager& getTransferManager() { return m_manager; }
private:
	QTreeWidgetItem* findOrCreateRoot(const QString& path);
	void deleteTreeItems(QTreeWidgetItem* item);
	void processUpdateTreeView(const std::vector<DirectoryEntry>& entries, const std::string& path);
	void treeWidgetSetClickedEnabled(bool flag) { m_treeWidgetLeftClickForbidden = flag; };

	void constructLocalTreeView();
	void constructRemoteTreeView();
	void constructTransferStatusWidget();


private:
	TreeView* m_treeView;
	TreeWidget* m_treeWidget;
	bool m_treeWidgetLeftClickForbidden = false;

	QLabel* m_sftpServerNameLabel;
	QLineEdit* m_sftpServerNameLineEdit;

	QLabel* m_sftpUserNameLabel;
	QLineEdit* m_sftpUserNameLineEdit;

	QLabel* m_sftpPasswordNameLabel;
	QLineEdit* m_sftpPasswordNameLineEdit;

	QLabel* m_localFileToUploadLabel;
	QLineEdit* m_localFileToUploadLineEdit;

	QLabel* m_localFolderLabel;
	QLineEdit* m_localFolderLineEdit;

	QLabel* m_remoteFileToUploadLabel;
	QLineEdit* m_remoteFileToUploadLineEdit;

	QLabel* m_remoteFolderLabel;
	QLineEdit* m_remoteFolderLineEdit;

	QPushButton* m_connectDisconnectButton;

	QString m_textCommandParameterLocal;
	QString m_textCommandParameterRemote;
	QString m_directoryNameLocal;
	QString m_directoryNameRemote;

	QTextEdit m_textDebugLog;
	QComboBox* m_logLevelComboBox;


	TransferManager m_manager;

	TreeWidget* m_transferStatusWidget;
	QMap<uint64_t, QTreeWidgetItem*> m_transferItems;

	QMutex m_mutex;

	QString m_remoteSourcePath;
	QString m_localSourcePath;
	QList<QString> m_expandedPaths;
	bool m_isCutOperation;
	bool m_isCutLocalOperation;
	bool m_isConnected = false;

	QMenu* m_LocalContextMenu;
	QMenu* m_RemoteContextMenu;
	QMenu* m_transferStatusContextMenu;

	// QActions for context menus
	QAction* m_downloadRemoteAction;
	QAction* m_copyRemoteAction;
	QAction* m_cutRemoteAction;
	QAction* m_pasteRemoteAction;
	QAction* m_deleteRemoteAction;
	QAction* m_uploadRemoteAction;
	QAction* m_deleteLocalAction;
	QAction* m_uploadLocalAction;
	QAction* m_cutLocalAction;
	QAction* m_copyLocalAction;
	QAction* m_pasteLocalAction;
	QAction* m_renameLocalAction;
	QAction* m_cancelAction;
	QAction* m_removeAction;
	QAction* m_pauseAction;
	QAction* m_resumeAction;

};

#endif // SFTP_CLIENT_MAINGUI_H
