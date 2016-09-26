all:	gpl-3.0.pdf spec.pdf
DSRC := src

gpl-3.0.pdf: $(DSRC)/gpl-3.0.tex
	latex $(DSRC)/gpl-3.0.tex
	latex $(DSRC)/gpl-3.0.tex
	dvips -q -z -t letter -P pdf -o gpl-3.0.ps gpl-3.0.dvi
	ps2pdf -dAutoRotatePages=/All gpl-3.0.ps gpl-3.0.pdf
	rm gpl-3.0.dvi gpl-3.0.log gpl-3.0.aux gpl-3.0.ps

spec.pdf: $(DSRC)/spec.tex $(DSRC)/gqtekspec.cls $(DSRC)/GT.eps
	cd $(DSRC)/; latex spec.tex
	cd $(DSRC)/; latex spec.tex
	cd $(DSRC)/; dvips -q -z -t letter -P pdf -o ../spec.ps spec.dvi
	ps2pdf -dAutoRotatePages=/All spec.ps spec.pdf
	-grep -i warning $(DSRC)/spec.log
	@rm -f $(DSRC)/spec.dvi $(DSRC)/spec.log
	@rm -f $(DSRC)/spec.aux $(DSRC)/spec.toc
	@rm -f $(DSRC)/spec.lot $(DSRC)/spec.lof
	@rm -f $(DSRC)/spec.out spec.ps

.PHONY: clean
clean:
	rm -f $(DSRC)/spec.dvi $(DSRC)/spec.log
	rm -f $(DSRC)/spec.aux $(DSRC)/spec.toc
	rm -f $(DSRC)/spec.lot $(DSRC)/spec.lof
	rm -f $(DSRC)/spec.out spec.ps spec.pdf
	rm -f gpl-3.0.pdf

