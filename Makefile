THEME = "beige"
TRANSITION = "linear"

all: slides.md
	pandoc -t html5 --template=template-revealjs.html --standalone --section-divs --variable theme=$(THEME) --variable transition=$(TRANSITION) slides.md -o slides.html
	cp slides.html reveal.js/slides.html

read:
	firefox reveal.js/slides.html
