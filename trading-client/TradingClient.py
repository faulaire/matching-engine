from TCPClient import *


class TradingClient(TCPClient):

    current_client_order_id = 1

    def __init__(self, host ,port):
        TCPClient.__init__(self, host, port)
        self.send_logon_msg()

    def send_logon_msg(self):
        message = protocol_pb2.OneMessage()
        message.type = protocol_pb2.OneMessage.LOGON

        message.logon_msg.user_name = "fabien"
        message.logon_msg.password = "aulaire"

        self.send_msg(message)

    def send_new_order(self, price, qty, side, instrument_id):
        message = protocol_pb2.OneMessage()
        message.type = protocol_pb2.OneMessage.NEW_ORDER
        message.new_order_msg.limit_price = price
        message.new_order_msg.order_quantity = qty
        message.new_order_msg.side = side
        message.new_order_msg.instrument_id = instrument_id
        message.new_order_msg.client_order_id = TradingClient.current_client_order_id
        TradingClient.current_client_order_id+=1

        self.send_msg(message)

    def handle_connect(self):
        logging.info("connected")

    def on_connection_closed(self):
        logging.info("on connection closed")
        self.is_logged = False

    def process_logon_reply(self, message):
        logging.info("process_logon_reply : %s" % message)
        self.is_logged = True

    def process_logout(self, message):
        logging.info("process_logon_reply : %s" % message)
        self.is_logged = False

    def process_heartbeat(self, message):
        logging.info("process_heartbeat")

    def process_execution_report(self, message):
        logging.info("process_execution_report : %s" % message)
