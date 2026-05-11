# Documentation Index

This directory contains the engineering documentation for the ProbeBridge.

## Core Documents

- [Architecture](architecture.md): firmware modules, data flow, and design tradeoffs.
- [Command Reference](command-reference.md): complete USB CDC command grammar and reply format.
- [Hardware Bring-Up](hardware-bringup.md): wiring, first power-on checks, and protocol validation.
- [Testing Strategy](testing.md): host parser tests, firmware smoke tests, and bench validation.
- [LaTeX Manual](latex/manual.tex): formal technical manual suitable for export to PDF.

## Intended Audience

This documentation is written for firmware engineers, test engineers, and hardware bring-up teams who need a small, inspectable bus adapter that can be modified quickly for lab automation.

## Documentation Build

If a LaTeX distribution is installed:

```powershell
pdflatex -output-directory docs\latex\build docs\latex\manual.tex
```

The Markdown files are the fastest path for day-to-day use. The LaTeX manual is intended for polished project submission, design review, or portfolio packaging.
