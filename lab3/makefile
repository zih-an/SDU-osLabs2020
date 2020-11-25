grade: testsh msh testinput.txt
	@rm -rf tmp
	@mkdir tmp
	@cp ./testsh tmp
	@cp ./msh tmp
	@cp testinput.txt tmp
	@echo + grading...
	@cd tmp && ./testsh ./msh

%: %.c
	@echo + gcc $<
	@gcc $< -o $@

clean:
	-rm -rf tmp testsh msh