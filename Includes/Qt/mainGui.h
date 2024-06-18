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
#include "WorkerThread.h"
#include "ThreadPool.h"
#include "TransferManager.h"
#include <qdrag.h>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QMimeData>

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
	void processTreeWidgetItemClicked(QTreeWidgetItem* item, int index);
	void eventFromThreadPoolReceived(int);
	void onDirectoryCacheUpdated(const std::string& path);
public:
	TreeViewWidget();
	void populateTreeView();
	void refreshTreeViewRoot(const std::string& path);
	void updateTreeView(const std::string& path);

	QTextEdit& getDebugLog() { return m_textDebugLog; }
	TransferManager& getTransferManager() { return m_manager; }
private:
	QTreeWidgetItem* findOrCreateRoot(const QString& path);

private:
	TreeView* m_treeView;
	TreeWidget* m_treeWidget;

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

	bool m_isConnected = false;

	TransferManager m_manager;
	QMutex m_mutex;
};

#endif // SFTP_CLIENT_MAINGUI_H
