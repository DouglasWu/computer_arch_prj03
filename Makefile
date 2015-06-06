TARGET=CMP

$(TARGET): main.o instruction.o hitmiss.o
	g++ -o $(TARGET) main.o instruction.o hitmiss.o

main.o: main.cpp parameter.h instruction.h hitmiss.h
	g++ -c main.cpp

instruction.o: instruction.cpp instruction.h hitmiss.h parameter.h
	g++ -c instruction.cpp
	
hitmiss.o: hitmiss.cpp hitmiss.h parameter.h
	g++ -c hitmiss.cpp

clean:
	rm -f $(TARGET) *.o *.bin *.rpt *.exe *.out
