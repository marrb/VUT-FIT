ProjName = proj3

run:
	latex $(ProjName).tex
	latex $(ProjName).tex
	dvips -t a4 $(ProjName).dvi
	ps2pdf -sPAPERSIZE=a4 $(ProjName).ps $(ProjName).pdf

	rm -f *.log *.aux *.ps *.dvi *.out
