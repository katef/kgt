
DIA ?= dia -t svg -e -

all: phases.svg

phases.svg: phases.dia
	# need a .svg file to satisfy rsvg-convert for image links
	${DIA} $< > phases-tmp.svg
	rsvg-convert -az .65 -f svg phases-tmp.svg > $@
	+rm phases-tmp.svg

