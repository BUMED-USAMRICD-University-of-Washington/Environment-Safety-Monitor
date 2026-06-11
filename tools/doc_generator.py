import os
import re
from pathlib import Path
import typer
from rich.console import Console
from rich.table import Table

# Initialize the Typer CLI app and Rich terminal layout console
app = typer.Typer(help="Interactive Documentation and Compliance Tool for Environment-Safety-Monitor")
console = Console()

def parse_cpp_constants(header_path: Path) -> dict:
    """Parses a C++ header file to extract constexpr values using regular expressions."""
    constants = {}
    if not header_path.exists():
        return constants
        
    # Regex to match: constexpr [type] [NAME] = [VALUE];
    pattern = re.compile(r"constexpr+\w+(?:_t)?+(\w+)*=*([^;]+);")
    
    with open(header_path, "r", encoding="utf-8") as f:
        for line in f:
            match = pattern.search(line)
            if match:
                name, val = match.groups()
                # Clean up comments or type casts from raw C++ string values
                val = val.split("//")[0].strip().replace("f", "")
                constants[name] = val
    return constants

@app.command()
def verify_thresholds(
    main_file: Path = typer.Option("src/main.cpp", help="Path to main runtime source file"),
    o2_file: Path = typer.Option("include/oxygen_sensor.h", help="Path to oxygen configuration file")
):
    """
    Scans the repository to display an engineering threshold matrix for facility inspection.
    """
    console.print("[bold blue]Scanning C++ Source Files for Live Environmental Thresholds...[/bold blue]\n")
    
    # Extract live configuration data from codebase sources
    main_data = parse_cpp_constants(main_file)
    o2_data = parse_cpp_constants(o2_file)
    
    table = Table(title="Fred Hutch Deployment Configuration Matrix")
    table.add_column("Parameter System Name", style="cyan", no_wrap=True)
    table.add_column("Active Code Value", style="magenta")
    table.add_column("Engineering Units", style="green")
    table.add_column("Safety Compliance Notes", style="yellow")
    
    table.add_row(
        "O2_CRITICAL_THRESHOLD", 
        main_data.get("O2_CRITICAL_THRESHOLD", "19.5"), 
        "%", 
        "OSHA Asphyxiation Evacuation limit. Circuit breaks below this point."
    )
    table.add_row(
        "TEMP_CRITICAL_THRESHOLD", 
        main_data.get("TEMP_CRITICAL_THRESHOLD", "0.0"), 
        "°C", 
        "Cryogenic flash floor sensor trigger floor limit."
    )
    table.add_row(
        "SAMPLE_INTERVAL_MS", 
        main_data.get("SAMPLE_INTERVAL_MS", "100"), 
        "ms", 
        "Deterministic loop sweep sampling delay rate."
    )
    table.add_row(
        "LOOP_VOLTAGE_MIN", 
        o2_data.get("LOOP_VOLTAGE_MIN", "1.0"), 
        "V", 
        "4mA Loop standard. Voltages below this value signal a physical wire break."
    )
    
    console.print(table)

@app.command()
def generate_readme(
    output_path: Path = typer.Option("README_COMPLIANCE.md", help="Where to output the formatted markdown text file")
):
    """
    Generates a standardized Markdown compliance report file for facility safety audits.
    """
    main_data = parse_cpp_constants(Path("src/main.cpp"))
    
    markdown_content = f"""# Environment Safety Monitor - Deployment Documentation
    
## System Overview
This repository manages a safety-critical micro-controller loop deployed to evaluate air quality and monitor cryogenic lines.

## Engineering Profile Specifications
- **OSHA Oxygen Trigger Point:** {main_data.get('O2_CRITICAL_THRESHOLD', '19.5')}%
- **Cryogenic Threshold Boundary:** {main_data.get('TEMP_CRITICAL_THRESHOLD', '0.0')}°C
- **Relay Loop Failure Response Timing:** {main_data.get('SAMPLE_INTERVAL_MS', '100')}ms

## Hardware Relay Interfacing Pattern
To achieve structural compliance with building alarm grids (e.g., **Edwards FireWorks**), hardware dry contacts must map to a **Normally Closed (NC)** safety topology loop. If power drops, software freezes, or watchdog triggers clear the registers, the loop will break instantly.
"""
    
    with open(output_path, "w", encoding="utf-8") as f:
        f.write(markdown_content.strip())
        
    console.print(f"[bold green]✓ Success![/bold green] Generated structured markdown compliance log: [underline]{output_path}[/underline]")

if __name__ == "__main__":
    app()
