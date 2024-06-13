#pragma once

#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QMenuBar>
#include <QtGui/QImage>
#include <QtWidgets/QMenu>
#include <QtWidgets/QSlider>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFileDialog>
#include <QtCore/qtimer.h>
#include <QtCore/QThread>
#include <QtCore/qfuture.h>
#include <QtConcurrent/qtconcurrentrun.h>
#include <QtCore/qdebug.h>
#include "QtWidgets/qinputdialog.h"
#include "QtCore/qsortfilterproxymodel.h"
#include "QtWidgets/qtableview.h"
#include "QtGui/QMouseEvent"
#include <QtWidgets/qwidgetaction.h>
#include "QtWidgets/qtablewidget.h"
#include "QtWidgets/qmessagebox.h"
#include "QtCore/qmetatype.h"
#include "QtCore/qmutex.h"
#include "QtCore/QMutexLocker"
#include "QtCore/qwaitcondition.h"
#include "QtNetwork/qnetworkaccessmanager.h"
#include "QtNetwork/qnetworkreply.h"
#include "QtNetwork/qnetworkrequest.h"
#include "QtWidgets/qtextedit.h"
#include "QtGui/qimagereader.h"
#include "QtGui/qimage.h"
#include "QtCore/qtimer.h"

#include <QtWidgets/QFileSystemModel>
#include <QtWidgets/QFileIconProvider>
#include <QtGui/QScreen>
#include <QtWidgets/QScroller>
#include <QtWidgets/QTreeView>
#include <QtWidgets/qtreewidget.h>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCommandLineOption>
#include <QtWidgets/qmenubar.h>
#include <QtNetwork/qtcpserver.h>

// STD threading
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <functional>
#include <vector>
#include <memory>
#include <queue>

#include "TransferManager.h"

class MyThread;

//Threadpool class
class ThreadPool : public QWidget {
	Q_OBJECT
signals:
	void WorkDone(int);
public:
	ThreadPool();
	~ThreadPool();
	void Start(uint32_t numThreads);
	void QueueJob(std::function<void()> job);
	void Stop();
	void ThreadLoop(MyThread* thread);

public slots:
	void EventFromThreadPoolReceived(int);

private:
	volatile std::atomic<bool> m_PoolStarted = false;
	bool m_ShouldTerminate = false;
	QMutex m_QueueMutex;
	QWaitCondition m_MutexCondition;
	std::vector<MyThread*> m_Threads;
	std::queue<std::function<void()>> m_Jobs;
};

//Basic thread
class MyThread : public QThread
{
	Q_OBJECT
signals:
	void WorkDone(int);
public:
	MyThread(ThreadPool* parent);
	void SetThreadId(int id) {
		m_ThreadId = id;
	}
	int& GetThreadId() {
		return m_ThreadId;
	}
private:
	ThreadPool* m_Parent;
	int m_ThreadId;
	void run();
};

//Overwritten QTreeView for mouse event
class TreeView : public QTreeView {
	Q_OBJECT
signals:
	void RightClickAction(QMouseEvent* event);
public:
	TreeView(QWidget* parent = nullptr);
	void mousePressEvent(QMouseEvent* event) override;
};

//Overwritten QTreeWidget for mouse event
class TreeWidget : public QTreeWidget {
	Q_OBJECT
signals:
	void RightClickAction(QMouseEvent* event);
public:
	TreeWidget(QWidget* parent = nullptr);
	void mousePressEvent(QMouseEvent* event) override;
};

class TreeViewWidget : public QWidget {
	Q_OBJECT
signals:
public slots:
	void OnClickedTreeView(const QModelIndex& index);
	void OnConnectButtonClicked();
	void OnRightClickedAction(QMouseEvent* event);
	void OnRightClickedActionTreeWidget(QMouseEvent* event);
	void ProcessTreeWidgetItemClicked(QTreeWidgetItem* item, int index);
	void EventFromThreadPoolReceived(int);
	void onDirectoryCacheUpdated(const std::string& path);
public:
	TreeViewWidget();
	void populateTreeView();
	void updateTreeView(const std::string& path);
private:
	QTreeWidgetItem* findOrCreateRoot(const QString& path);

private:
	TreeView* m_TreeView;
	TreeWidget* m_TreeWidget;

	QLabel* m_SftpServerNameLabel;
	QLineEdit* m_SftpServerNameLineEdit;

	QLabel* m_SftpUserNameLabel;
	QLineEdit* m_SftpUserNameLineEdit;

	QLabel* m_SftpPasswordNameLabel;
	QLineEdit* m_SftpPasswordNameLineEdit;

	QLabel* m_LocalFileToUploadLabel;
	QLineEdit* m_LocalFileToUploadLineEdit;

	QLabel* m_LocalFolderLabel;
	QLineEdit* m_LocalFolderLineEdit;

	QLabel* m_RemoteFileToUploadLabel;
	QLineEdit* m_RemoteFileToUploadLineEdit;

	QLabel* m_RemoteFolderLabel;
	QLineEdit* m_RemoteFolderLineEdit;

	QPushButton* m_ConnectDisconnectButton;

	QString m_TextCommandParameterLocal;
	QString m_TextCommandParameterRemote;
	QString m_DirectoryNameLocal;
	QString m_DirectoryNameRemote;

	QTextEdit m_TextDebugLog;

	bool m_IsConnected = false;

	ThreadPool m_ThreadPool;

	TransferManager m_manager;
};