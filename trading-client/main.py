from TradingClient import *

import sys
import mainwindow

from PyQt5.QtWidgets import QMainWindow, QApplication, QMessageBox

class TradingGUI(TradingClient,QMainWindow):

    def __init__(self, host, port):
        TradingClient.__init__(self, host, port)
        QMainWindow.__init__(self)

        self.ui = mainwindow.Ui_MainWindow()
        self.ui.setupUi(self)

        self.configureLogger()

        self.ui.pushButton.clicked.connect(self.connect)
        self.ui.actionQuit.triggered.connect(self.handleQuit)
        self.ui.SendButton.clicked.connect(self.handleSend)

    def configureLogger(self):

        class QTextBoxStream:
            def __init__(self, QTextBox):
                self.QTextBox = QTextBox

            def write(self, record):
                self.QTextBox.insertPlainText(record)

        logging.basicConfig(level=logging.DEBUG)

        # TODO Change formatter to check if it's an error
        # TODO Add an handler to write logs into a file
        # TODO Disable connect button if already connected
        # TODO Add a disconnect button

        handler = logging.StreamHandler( stream=QTextBoxStream(QTextBox=self.ui.textBrowser))
        handler.setFormatter(fmt=logging.Formatter('%(asctime)-15s %(message)s'))

        root_logger = logging.getLogger('')
        # Remove existing handlers
        root_logger.handlers = []
        root_logger.addHandler(handler)


    def handleSend(self):
        try:
            price = float(self.ui.PricelineEdit.text())
            qty = int(self.ui.QtylineEdit.text())
            instrument_id = int(self.ui.InstrumentIDlineEdit.text())
            side = self.ui.SidecomboBox.currentText()
            logging.info("%s %s@%s %s" % (side, qty, price, instrument_id))
            side = protocol_pb2.NewOrder.BUY if side == "BUY" else protocol_pb2.NewOrder.SELL
            self.send_new_order(price, qty, side, instrument_id)
        except Exception as e:
            logging.error(e)


    def handleConnect(self):
        self.connect()

    def handleQuit(self):
        if QMessageBox.question(None, '', "Are you sure you want to quit?",
                                QMessageBox.Yes | QMessageBox.No,
                                QMessageBox.No) == QMessageBox.Yes:
            QApplication.closeAllWindows(self)


if __name__ == "__main__":
    app = QApplication(sys.argv)

    w = TradingGUI('127.0.0.1', 5001)
    w.show()

    while w.isVisible():
        app.processEvents()
        w.process_idle()
        asyncore.loop(timeout=0, use_poll=False, count=1)