TARGET=pipeline

$(TARGET):main.o 
	gcc main.o -o $(TARGET)
main.o:main.c simulator.h pipelineregisters.h controlunit.h pipeline.h errordetect.h stagefunction.h
	gcc -c main.c
clean:
	rm -f $(TARGET) *.o *.bin *.rpt *.exe *.out
