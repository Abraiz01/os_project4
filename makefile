all:
	gcc adtar.c -o adtar

adtar: adtar.c
	gcc adtar.c -o adtar

clean:
	rm -f adtar  *.o