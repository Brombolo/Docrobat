# Docrobat - Editor PDF Professionale per Haiku OS

**Docrobat** è un editor e visualizzatore PDF professionale e nativo per **Haiku OS** (x86_64), progettato per integrarsi armoniosamente con l'esperienza desktop di Haiku, rispettandone le convenzioni di design, la reattività e le API di sistema.

Il software è scritto in **C++20** ed è costruito esclusivamente sui kit nativi di Haiku (`AppKit`, `InterfaceKit`, `StorageKit`, `TranslationKit`) abbinati al motore di rendering e manipolazione documentale **`poppler-cpp`**.

---

## Architettura Modulare Professionale

Docrobat adotta un'architettura modulare a livelli, orientata alle prestazioni e all'estensibilità:

```
+-------------------------------------------------------------------------+
|                              DocrobatApp                                |
+-------------------------------------------------------------------------+
|                               MainWindow                                |
|  +--------------------+-----------------------------------------------+  |
|  |  Sidebar (BTabView) |          Area Documento (BScrollView)         |  |
|  |  - Miniature       |                   PDFView                     |  |
|  |  - Segnalibri/Ind. |   - BBitmap Rendering (B_RGBA32)              |  |
|  |  - Moduli/AcroForm |   - Live Annotation Overlay                   |  |
|  |  - Sicurezza/Prop. |   - Pan & Interactive Tools                   |  |
|  +--------------------+-----------------------------------------------+  |
+-------------------------------------------------------------------------+
|                            Moduli Core                                  |
|   PDFDocument   |  PDFObjectEditor  |  PDFFormManager  |  PDFSecurity   |
+-------------------------------------------------------------------------+
|                             Motore PDF                                  |
|                             poppler-cpp                                 |
+-------------------------------------------------------------------------+
```

### Moduli Principali:

1. **`PDFDocument`**:
   - Gestione strutturale del documento (conteggio pagine, dimensioni in punti, orientamento).
   - Rotazione pagine (90°, 180°, 270°), cancellazione e riordinamento.
   - Rasterizzazione ad alta definizione su Haiku `BBitmap` (`B_RGBA32`) con antialiasing del testo.
   - Salvataggio e persistenza delle modifiche e dei metadati mediante gli attributi estesi nativi del filesystem Haiku BFS (`BNode::WriteAttr`).

2. **`PDFObjectEditor`**:
   - Ispezione e rilevamento di blocchi di testo vettoriali e caselle di delimitazione.
   - Rilevamento di oggetti immagine incorporati nelle pagine.
   - Estrazione di immagini in istanze native `BBitmap` e sostituzione selettiva.
   - Redazione e rimozione visiva di elementi indesiderati.

3. **`PDFFormManager`**:
   - Gestione completa dei campi modulo interattivi (AcroForms: caselle di testo, caselle di spunta, menu a discesa).
   - Binding dinamico con controlli grafici nativi di Haiku (`BTextControl`, `BCheckBox`, `BPopUpMenu`) posizionati sopra la vista della pagina.

4. **`PDFSecurity`**:
   - Rilevamento crittografia documento e verifica password (apertura e proprietario).
   - Ispezione, rimozione e applicazione di restrizioni e permessi (stampa, modifica, copia, compilazione moduli).

5. **`PDFView`**:
   - Vista ad alte prestazioni con disegno diretto via `DrawBitmap()` e calcolo dinamico dell'ombra del foglio.
   - Strumenti interattivi con overlay in tempo reale:
     - ✋ **Vista:** Pan dragging e scorrimento rapido con mouse e scorciatoie da tastiera.
     - 🖊 **Evidenziatore:** Tracciamento semitrasparente in alpha blending (`B_OP_ALPHA`).
     - 📝 **Nota di testo:** Inserimento note e commenti posizionati.
     - ▭ **Rettangolo:** Tracciamento forme geometriche e bordi di evidenziazione.

6. **`MainWindow` & Barra Laterale a Schede (`BTabView`)**:
   - Barra superiore con controlli di navigazione pagine, contatore "Pagina X di Y" e zoom (25% - 500%, Adatta pagina).
   - Seconda barra strumenti dedicata all'editing rapido e selezione strumento.
   - Pannello laterale collassabile (`BSplitView`) organizzato in 4 schede:
     - **Pagine:** Elenco e navigazione immediata delle pagine e dimensioni.
     - **Segnalibri:** Albero gerarchico dell'indice del documento (`BOutlineListView`).
     - **Moduli:** Lista e compilazione rapida dei campi AcroForms.
     - **Sicurezza:** Riepilogo metadati, cifratura e controlli di sblocco/protezione.

---

## Requisiti e Dipendenze

Per compilare Docrobat su Haiku OS (x86_64) sono necessari i seguenti pacchetti di sviluppo:

- `gcc` (supporto completo a C++20)
- `cmake` (versione 3.16 o superiore)
- `make`
- `poppler_devel` (fornisce `libpoppler-cpp`)
- `haiku_devel` (librerie di sistema `libbe`, `libtracker`, `libtranslation`)

Puoi installare le dipendenze richieste tramite `pkgman` da Terminale:

```bash
pkgman refresh
pkgman install gcc cmake make poppler_devel
```

---

## Compilazione da Sorgente

Per compilare ed eseguire Docrobat su Haiku OS:

```bash
# Clona il repository
git clone https://github.com/Brombolo/Docrobat.git
cd Docrobat

# Configura il build con CMake
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Compila l'eseguibile
cmake --build build

# Avvia Docrobat
./build/Docrobat
```

---

## Licenza

Distribuito sotto licenza MIT. Consulta il file `LICENSE` per ulteriori dettagli.
