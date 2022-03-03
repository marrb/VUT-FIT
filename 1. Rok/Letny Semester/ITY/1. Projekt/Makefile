ProjName = proj1

run:
	latex $(ProjName).tex
	dvips -t a4 $(ProjName).dvi
	ps2pdf $(ProjName).ps $(ProjName).pdf

	rm -f *.log *.aux *.ps *.dvi *.out
