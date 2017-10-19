# coding=utf-8  
import sys
import time
from PyQt4.QtCore import *
from PyQt4.QtGui import *
import urllib.request

ip=None

class WorkThread(QThread):
    trigger = pyqtSignal(bytes)

    def __int__(self):
        super(WorkThread, self).__init__()

    def run(self):
        while True:
          try:
            fp = urllib.request.urlopen("http://"+ip+"/get")
            bimg = fp.read()
          except:
            pass
          finally:
            fp.close()
            self.trigger.emit(bimg)

class App(QWidget):
    def __init__(self):
        super().__init__()
        self.title = 'PyQt4 image - pythonspot.com'
        self.left = 100
        self.top = 100
        self.width = 320
        self.height = 240
        self.initUI()

    def initUI(self):
        self.setWindowTitle(self.title)
        self.setGeometry(self.left, self.top, self.width, self.height)

        layout = QVBoxLayout(self)

        qIm = QImage(bytearray(320 * 240), 320, 240, QImage.Format_Indexed8)
        self.label = QLabel()
        pixmap = QPixmap(qIm)
        self.label.setPixmap(pixmap)
        layout.addWidget(self.label)

        self.editor = QLineEdit("192.168.0.107")
        layout.addWidget(self.editor)

        self.button = QPushButton("Start")
        layout.addWidget(self.button)

        self.workThread = WorkThread()

        self.button.clicked.connect(self.work)

        self.resize(320, 240)
        self.show()

    def flush(self,bimg):
        print("flush")
        print(bimg)
        qIm = QImage(bimg, 320, 240, QImage.Format_Indexed8)
        pixmap = QPixmap(qIm)
        self.label.setPixmap(pixmap)

    def work(self):
        global  ip
        ip = self.editor.text()
        print(ip)
        self.workThread.start()
        self.workThread.trigger.connect(self.flush)

if __name__ == '__main__':
    app = QApplication(sys.argv)
    ex = App()
    sys.exit(app.exec_())
