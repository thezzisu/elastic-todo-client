#include <QApplication>
#include <QProcess>
#include <QWebEngineView>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  QWebEngineView view;
  QObject::connect(&view, &QWebEngineView::iconChanged,
                   [&view]() { view.window()->setWindowIcon(view.icon()); });
  QObject::connect(&view, &QWebEngineView::titleChanged,
                   [&view]() { view.window()->setWindowTitle(view.title()); });
  view.load(QUrl("https://elastic-todo.pages.dev"));
  view.resize(1024, 750);
  view.show();
  return a.exec();
}
