# LaTeX Manual

`manual.tex` is a formal technical manual for the ProbeBridge.

Build with:

```powershell
pdflatex -output-directory docs\latex\build docs\latex\manual.tex
```

If the output directory does not exist, create it first:

```powershell
New-Item -ItemType Directory -Force docs\latex\build
```
