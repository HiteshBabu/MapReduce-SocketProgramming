all:
	gcc -o client client.c -lsocket -lnsl -lresolv
	gcc -o aws.out aws.c -lsocket -lnsl -lresolv
	gcc -o serverA.out serverA.c -lsocket -lnsl -lresolv
	gcc -o serverB.out serverB.c -lsocket -lnsl -lresolv
	gcc -o serverC.out serverC.c -lsocket -lnsl -lresolv
.PHONY: serverA serverB serverC aws
serverA:
	./serverA.out
serverB:
	./serverB.out
serverC:
	./serverC.out
aws:
	./aws.out
