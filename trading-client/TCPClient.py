import asyncore, socket
import protocol_pb2
import struct
import time
import logging


class TCPClient(asyncore.dispatcher):

    message_type = {
                        protocol_pb2.OneMessage.LOGON_REPLY:      ("process_logon_reply", "logon_reply_msg"),
                        protocol_pb2.OneMessage.LOGOUT:           ("process_logout", "logout_msg"),
                        protocol_pb2.OneMessage.HEARTBEAT:        ("process_heartbeat", "heartbeat_msg"),
                        protocol_pb2.OneMessage.EXECUTION_REPORT: ("process_execution_report", "execution_report_msg")
                   }

    def __init__(self, host, port):
        asyncore.dispatcher.__init__(self)
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.host = host
        self.port = port
        self.output_buffer = bytes()
        self.input_buffer = bytes()
        self.is_logged = False
        self.last_msg_sent_timestamp = 0

    def connect(self):
        logging.info("connecting to %s:%d" % (self.host, self.port))
        super().connect((self.host, self.port))

    def handle_close(self):
        logging.info("connection closed")
        self.is_logged = False
        self.close()
        self.on_connection_closed()

    def handle_read(self):
        self.input_buffer = self.input_buffer + self.recv(64)
        while self.try_decode_message():
            pass

    def handle_error(self):
       logging.error("handle_error")

    def try_decode_message(self):
        buffer_length = len(self.input_buffer)
        if buffer_length > 4:
            message_length = struct.unpack('I', self.input_buffer[:4])
            message_length = int(message_length[0])
            message_offset = message_length + 4
            if (message_offset) <= buffer_length:
                message = protocol_pb2.OneMessage()
                message.ParseFromString(self.input_buffer[4:message_offset])
                self.decode_functional_message(message)
                self.input_buffer = self.input_buffer[message_offset:]
                return True
        else:
            return False


    def decode_functional_message(self, message):
        try:
            (callback_name, attr_name) = TCPClient.message_type[message.type]
            if message.HasField(attr_name):
                callback = getattr(self, callback_name)
                callback(getattr(message, attr_name))
            else:
                logging.error("Error : Field %s not available in message"%attr_name)
        except KeyError:
            logging.info("Unhandled message type : %d"%message.type)
        except Exception as e:
            print(e)

    def writable(self):
        return (len(self.output_buffer) > 0)

    def handle_write(self):
        sent = self.send(self.output_buffer)
        self.output_buffer = self.output_buffer[sent:]
        self.last_msg_sent_timestamp = int(time.time());

    def send_heartbeat(self):
        message = protocol_pb2.OneMessage()
        message.type = protocol_pb2.OneMessage.HEARTBEAT
        message.heartbeat_msg.SetInParent()
        self.send_msg(message)

    def process_idle(self):
        if self.is_logged:
            now = int(time.time())
            if now - self.last_msg_sent_timestamp >= 5:
                self.send_heartbeat()

    def send_msg(self, message):
        buffer = message.SerializeToString()
        self.output_buffer = self.output_buffer + struct.pack('I', len(buffer)) + bytes(buffer)