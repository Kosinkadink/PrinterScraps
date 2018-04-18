
# Wait for successful serial connection
def wait_for_serial_connection(serial_conn):
	connected = False
	while not connected:
		serial_conn.read()
		connected = True
