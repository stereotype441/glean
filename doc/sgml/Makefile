FILES = glean.sgml

TXT_FILES  = $(FILES:.sgml=.txt)
DVI_FILES  = $(FILES:.sgml=.dvi)
PS_FILES   = $(FILES:.sgml=.ps)
HTML_FILES = $(FILES:.sgml=.html)

all:	txt ps html

txt:	$(TXT_FILES)
dvi:	$(DVI_FILES)
ps:	$(PS_FILES)
html:	$(HTML_FILES)

%.txt: %.sgml
	sgml2txt $*

%.dvi: %.sgml
	sgml2latex --papersize=letter $*

%.ps: %.sgml
	sgml2latex --papersize=letter --output=ps $*

%.html: %.sgml
	sgml2html --split=0 $*

clean:
	rm -f $(TXT_FILES) $(DVI_FILES) $(PS_FILES) $(HTML_FILES) *.ps *.png *.bak
