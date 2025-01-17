#include "Display/CMainWindow.h"

#include <QApplication>
#include <QtXml>

#include "Display/CEventFilterObj.h"
#include "Model/CConfigManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    CMainWindow *window = new CMainWindow;

	CConfigManager* confManager;
	confManager	= CConfigManager::getInstance();

    CXmlStorageHandler xmlStorageHandler;

    CProjectManager::getInstance()->setProjectFile(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/" + "record.xml");
    CProjectManager::getInstance()->setStorageHandler(xmlStorageHandler);
    CProjectManager::getInstance()->attachStorage();

    // load profile list after storage ready
	window->loadProjectList();

	QByteArray savedGeometry;

	savedGeometry = confManager->getValue("Window", "geometry").toByteArray();
	window->restoreGeometry(savedGeometry);

	QString splitterSizeStr;
	QStringList splitterSizeStrList;
	QList<int> splitterSizeList;
	
	QString vsplitterSizeStr;
	QStringList vsplitterSizeStrList;
	QList<int> vsplitterSizeList;

    // tab widget
    window->restoreTabWidgetPos();

	// splitter
    splitterSizeStr = confManager->getValue("Window", "splitter").toString();
    splitterSizeStr = splitterSizeStr.trimmed();
	if (splitterSizeStr != "") {
		splitterSizeStrList = splitterSizeStr.split(" ", Qt::SkipEmptyParts);

		foreach (const QString& splitterSize, splitterSizeStrList) {
			splitterSizeList.append(splitterSize.toInt());
		}

		window->setSplitterSizes(splitterSizeList);
	}
	
	// vertical splitter
    vsplitterSizeStr = confManager->getValue("Window", "vsplitter").toString();
    vsplitterSizeStr = vsplitterSizeStr.trimmed();
	
	if (vsplitterSizeStr != "") {
		vsplitterSizeStrList = vsplitterSizeStr.split(" ", Qt::SkipEmptyParts);

		foreach (const QString& splitterSize, vsplitterSizeStrList) {
			vsplitterSizeList.append(splitterSize.toInt());
		}

		window->setVerticalSplitterSizes(vsplitterSizeList);
	}

    qDebug() << window->geometry();
    qDebug() << window->frameGeometry();

    window->show();

	// event filter for show/hide menuBar
	CEventFilterObj *keyPressObj = new CEventFilterObj();
	app.installEventFilter(keyPressObj);

    return app.exec();
}

