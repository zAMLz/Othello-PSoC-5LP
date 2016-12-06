CFLAGS=-Wall -g

all:
	make pdf
	make clean
	mupdf lab_report.pdf &

pdf:
	pdflatex --shell-escape lab_report.tex

clean:
	rm -fv *.aux
	rm -fv *.log
	rm -fv *.dvi
	rm -fv *.out
	rm -rfv _minted*/
