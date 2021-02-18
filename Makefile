.PHONY=clean

all:
	@cd klient && make --no-print-directory
	@cd producent && make --no-print-directory
	@cd producent && ./producent 127.0.0.1:12346 -p 12800
	@#cd klient && ./klient 127.0.0.1:12346 -c 10 -d 20 -p 30
	@#cd producent && ./producent 65535 -p 4.5

clean:
	rm klient/klient producent/producent