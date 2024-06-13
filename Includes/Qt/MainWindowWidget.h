//#include <QWidget>
//#include <QTreeView>
//#include <QTreeWidget>
//#include <QLabel>
//#include <QLineEdit>
//#include <QPushButton>
//#include <QTextEdit>
//#include <QModelIndex>
//#include <QMouseEvent>
//#include <QTreeWidgetItem>
//#include <QString>
//#include <QVBoxLayout>
//
//#include "WorkerThread.h"
//#include "ThreadPool.h"
//#include "TreeView.h"
//
////Overwritten QTreeWidget for mouse event
//class TreeWidget : public QTreeWidget {
//	Q_OBJECT
//signals:
//	void RightClickAction(QMouseEvent* event);
//public:
//	TreeWidget(QWidget* parent = nullptr);
//	void mousePressEvent(QMouseEvent* event) override;
//};
//
//class TreeViewWidget : public QWidget {
//	Q_OBJECT
//signals:
//public slots:
//	void onClickedTreeView(const QModelIndex& index);
//	void onConnectButtonClicked();
//	void onRightClickedAction(QMouseEvent* event);
//	void onRightClickedActionTreeWidget(QMouseEvent* event);
//	void processTreeWidgetItemClicked(QTreeWidgetItem* item, int index);
//	void eventFromThreadPoolReceived(int);
//	void onDirectoryCacheUpdated(const std::string& path);
//public:
//	TreeViewWidget();
//	void populateTreeView();
//	void updateTreeView(const std::string& path);
//private:
//	QTreeWidgetItem* findOrCreateRoot(const QString& path);
//
//private:
//	TreeView* m_TreeView;
//	TreeWidget* m_TreeWidget;
//
//	QLabel* m_SftpServerNameLabel;
//	QLineEdit* m_SftpServerNameLineEdit;
//
//	QLabel* m_SftpUserNameLabel;
//	QLineEdit* m_SftpUserNameLineEdit;
//
//	QLabel* m_SftpPasswordNameLabel;
//	QLineEdit* m_SftpPasswordNameLineEdit;
//
//	QLabel* m_LocalFileToUploadLabel;
//	QLineEdit* m_LocalFileToUploadLineEdit;
//
//	QLabel* m_LocalFolderLabel;
//	QLineEdit* m_LocalFolderLineEdit;
//
//	QLabel* m_RemoteFileToUploadLabel;
//	QLineEdit* m_RemoteFileToUploadLineEdit;
//
//	QLabel* m_RemoteFolderLabel;
//	QLineEdit* m_RemoteFolderLineEdit;
//
//	QPushButton* m_ConnectDisconnectButton;
//
//	QString m_TextCommandParameterLocal;
//	QString m_TextCommandParameterRemote;
//	QString m_DirectoryNameLocal;
//	QString m_DirectoryNameRemote;
//
//	QTextEdit m_TextDebugLog;
//
//	bool m_IsConnected = false;
//
//	ThreadPool m_ThreadPool;
//
//	TransferManager m_manager;
//};