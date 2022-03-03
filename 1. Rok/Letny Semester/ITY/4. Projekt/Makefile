SHELL=/bin/sh
ProjName=proj4

.SUFFIXES:
.SUFFIXES: .bib .pdf .tex
.PHONY: clean

run: $(ProjName).pdf

$(ProjName).pdf: $(ProjName).bbl $(ProjName).tex
	pdflatex $(ProjName).tex -draftmode
	pdflatex $(ProjName).tex 

$(ProjName).bbl: $(ProjName).aux
	bibtex $(ProjName).aux

$(ProjName).aux: $(ProjName).bib
	pdflatex $(ProjName).tex -draftmode
	pdflatex $(ProjName).tex -draftmode

run: clean
clean:
	rm *.aux *.log *.bbl *.blg *.out