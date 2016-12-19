try:
	import readline
except ImportError:
	print "Module readline not available."
else:
	import rlcompleter
	readline.parse_and_bind('tab:complete')

import sys

from instrument_editor import *

def display_all_instruments(instrument_manager):
	instrument_manager.LoadInstruments()
	for key in instrument_manager.instruments:
		print instrument_manager.instruments[key]
	
def write_instrument(instrument_manager, name, isin, product_id, currency="EUR", close_price=0):
	Instr = Instrument(name, isin, currency, product_id, close_price)
	instrument_manager.WriteInstrument(Instr)
	
if __name__ == "__main__":
	if len(sys.argv) != 2:
		print "Error : Usage : %s : leveldb_database_path"%(sys.argv[0])
		sys.exit(1)
	
	leveldb_database_path = sys.argv[1]
	
	InstrMgr = InstrumentManager(leveldb_database_path)
	display_all_instruments(InstrMgr)
	
	