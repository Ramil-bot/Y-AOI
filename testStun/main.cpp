#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QMessageBox>
#include <iostream>


void onClick();

int main(int argc,char *argv[]) {
  
  QApplication app(argc, argv);
  
QWidget widget;
widget.setMinimumSize(250, 150);
widget.setWindowTitle("Hello world");
QPushButton btn{"Click", &widget};

QObject::connect(&btn, &QPushButton::clicked, onClick);
widget.show();
return app.exec();
  std::cout << "Hello world" << std::endl;
}

void onClick() {
  QMessageBox msgBox;
  msgBox.setText("Do you want me?");
  msgBox.exec();
}

