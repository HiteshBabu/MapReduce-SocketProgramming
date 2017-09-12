all:
	gcc -o client client.c
	gcc -o aws.out aws.c -lpthread
	gcc -o serverA.out serverA.c
	gcc -o serverB.out serverB.c
	gcc -o serverC.out serverC.c 
.PHONY: serverA serverB serverC aws
serverA:
	./serverA.out
serverB:
	./serverB.out
serverC:
	./serverC.out
aws:
	./aws.out
