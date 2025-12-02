build-sharpen: ./a.out

./a.out: image_sharpener.cpp libppm.cpp
	g++ -g image_sharpener.cpp libppm.cpp -o a.out

run-sharpen: ./a.out
	./a.out ../images/$(INPUT).ppm ../images/$(OUTPUT).ppm

run-sharpen-avg: ./a.out
	bash file.sh $(INPUT)

clean:
	rm -f a.out
