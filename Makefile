include config
export DOXYGEN_OUTPUT DOXYGEN_INPUT

# Compiler l'application
$(APPNAME): $(OBJ)
	$(CC) $(FLAGS) -o $@ $^

# Créer les fichiers objets des fichiers .c
%.o:
	@ $(shell mkdir -p $(OBJDIR))
	$(CC) -o $@ -c $(@:$(OBJDIR)%.o=$(SRCDIR)%.c)


.PHONY: run clean valgrind

# Lancer l'application a partir d'une commande prédéfinie
run: $(APPNAME)
	@ echo
	$(RUN_CMD)

# Nettoyer le projet
clean:
	rm $(OBJ) $(APPNAME)

valgrind: $(APPNAME)
	valgrind --leak-check=full ./$(APPNAME)


.PHONY: doc
# Nécessaire pour utilisation dans le fichier config Doxygen
export DOXYGEN_PNAME DOXYGEN_OUTPUT DOXYGEN_INPUT

# Lancer la création de la documentation
doc: $(APPNAME)
	@ doxygen $(DOXYGEN_CONFIG)
	@ cd $(DOXYGEN_OUTPUT)/latex && make
	@ cp $(DOXYGEN_OUTPUT)/latex/refman.pdf $(DOXYGEN_OUTPUT)/$(DOXYGEN_PDFNAME)