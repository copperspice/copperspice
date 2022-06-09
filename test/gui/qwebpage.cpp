#include "catch2/catch.hpp"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning(disable : 4251 4244 4250 4275)
#endif

#include <QString>
#include <QMainWindow>
#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>

#include <QVBoxLayout>
#include <QtWebkit/QWebView>
#include <QMdiSubWindow>
#include <QMdiArea>
#include <QObject>

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include "stopwatch.h"

#include <thread>
#include <chrono>
#include <iostream>

using namespace std::chrono_literals;

QUrl url("file:///D:/project2/kitchensink/crash_cs.html");

TEST_CASE("Webbrowser open url", "[q]") // [QWebView] // [.] prevents it from running by default
{
	Stopwatch maintimer("main");

	Stopwatch apptimer("app");
	QApplication app(__argc, __argv);
	apptimer.stop();

	Stopwatch QMainWindowTimer("QMainWindow");
	QMainWindow w;
	QMainWindowTimer.stop();

	w.resize(400, 300);
	w.setWindowTitle("Hello Copperspice");
	

	auto menubar = w.menuBar();
	auto menu = menubar->addMenu("Test");
	auto openAction = menu->addAction("Open browser");

	//QWebView* webview = new QWebView(&w);
	//w.setCentralWidget(webview);
	//webview->show();

	Stopwatch populate("WindowShow");
	w.show();
	populate.stop();


	Stopwatch WindowShow("WindowShow");
	w.show();
	WindowShow.stop();

	//{
	//	Stopwatch timer("load");
	//	webview->load(url);
	//}
	
	w.close();
	/*QObject::connect(openAction, &QAction::triggered, &w, [webview]
		{
			webview->load(url);
		});
	app.exec();*/
}