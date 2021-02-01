.PHONY=clean

all:
	@cd klient && make --no-print-directory
	@cd producent && make --no-print-directory
	# @cd producent && ./producent 120.21.12.1:1234 -p 4.5
	@cd producent && ./producent 1234 -p 4.5

clean:
	rm klient producent