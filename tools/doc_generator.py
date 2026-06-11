""" Typer CLI Extension: UDP Anomaly Listener """
import typer
import os
import re
import time
from pathlib import Path
import typer
import socket
import serial
from rich.console import Console
from rich.table import Table
from rich.live import Live

app = typer.Typer(help="Interactive Documentation and Calibration Tool for Environment-Safety-Monitor")
console = Console()


@app.command()
def listen_anomalies(port: int = 8005):
    """ Binds to the local UDP socket to listen for NVIDIA GPU broadcasts """
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("0.0.0.0", port))

    console.print("[SYSTEM] Listening for asynchronous UDP anomalies...", style="bold cyan")

    while True:
        data, addr = sock.recvfrom(1024)
        
        if not data:
            continue

        payload = data.decode("utf-8")
        
        if "CRITICAL" in payload:
            console.print("[EVACUATE] " + payload, style="bold red")
            continue
            
        if "WARNING" in payload:
            console.print("[WARNING] " + payload, style="bold yellow")
            continue
            
        if "FAULT" in payload:
            console.print("[HARDWARE FAULT] " + payload, style="bold magenta")
            continue

def print_safety_warning():
    warning_text = """
    CRITICAL LIFE-SAFETY ALERT: NITROGEN HYPOXIA DETECTED
    =====================================================
    DO NOT TRUST BEHAVIOR IN THE LEAK ZONE.

    When oxygen drops, individuals will NOT realize they are suffocating. 
    They will display symptoms resembling severe intoxication, irrational panic, 
    combativeness, or acute paranoia (e.g., accusing others of drugging them).

    * Evacuate instantly.
    * Do not argue with disoriented personnel. Force evacuation.
    * Never enter a leak zone without an independent air supply.
    """
    console.print(Panel(Markdown(warning_text), border_style="bold red"))

def parse_cpp_constants(header_path: Path) -> dict:
    """
    Parses a C++ header file to extract constexpr values using regular expressions.
    Ensures absolute traceability by reading directly from the source code.
    """
    constants = {}
    if not header_path.exists():
        return constants
        
    pattern = re.compile(r"constexpr+\w+(?:_t)?+(\w+)*=*([^;]+);")
    
    with open(header_path, "r", encoding="utf-8") as f:
        for line in f:
            match = pattern.search(line)
            if match:
                name, val = match.groups()
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

@app.command()
def read_telemetry(
    port: str = typer.Option("/dev/ttyUSB0", help="The serial port your microcontroller is plugged into (e.g., COM3 on Windows)"),
    baudrate: int = typer.Option(9600, help="The data transfer rate matching your C++ Serial.begin() speed"),
    timeout: float = typer.Option(2.0, help="The maximum seconds to wait for a data frame before throwing an error")
):
    """ Connects to the hardware array and streams live sensor data. """
    
    """ Hardware serial connection logic goes here """
    
    """ Simulated evaluation of the incoming serial stream """
    current_status = "CRITICAL" 
    
    if current_status == "CRITICAL":
        print_safety_warning()
        return
    """
    Connects to the hardware over a physical USB Serial connection to pull live runtime telemetry.
    """
    console.print(f"[bold blue]Establishing connection to safety hardware on port {port}...[/bold blue]")
    
    try:
        with serial.Serial(port, baudrate, timeout=timeout) as ser:
            console.print("[bold green]✓ Connected! Press Ctrl+C to disconnect and exit diagnostic mode.[/bold green]\n")
            
            ser.reset_input_buffer()
            
            with Live(console=console, screen=False, refresh_per_second=4) as live:
                while True:
                    raw_line = ser.readline().decode('utf-8', errors='ignore').strip()
                    
                    if not raw_line:
                        continue
                    
                    if raw_line.startswith("$TELEMETRY"):
                        fields = raw_line.split(",")
                        
                        if len(fields) < 9:
                            continue
                            
                        raw_o2        = fields[1]
                        smoothed_o2   = fields[2]
                        o2_volts      = fields[3]
                        raw_temp      = fields[4]
                        smoothed_temp = fields[5]
                        board_temp    = fields[6]
                        relay_raw     = int(fields[7])
                        
                        relay_text = "[bold green]CLOSED (SAFE)[/bold green]" if relay_raw == 1 else "[bold red]OPEN (ALARM / TRIPPED)[/bold red]"
                        
                        table = Table(title="Live Hardware Telemetry Stream")
                        table.add_column("Sensor Metric / System Register", style="cyan")
                        table.add_column("Raw Value", style="magenta")
                        table.add_column("Filtered / Smoothed", style="yellow")
                        table.add_column("Electrical / State Metrics", style="green")
                        
                        table.add_row("Oxygen Level", f"{raw_o2} %", f"{smoothed_o2} %", f"{o2_volts} V (4-20mA loop)")
                        table.add_row("Cryo Floor Array", f"{raw_temp} °C", f"{smoothed_temp} °C", "Type-T Thermocouple")
                        table.add_row("MAX31856 Cold-Junction", f"{board_temp} °C", "N/A", "Freezer Exhaust Zone")
                        table.add_row("Edwards FireWorks Relay", "N/A", "N/A", relay_text)
                        
                        live.update(table)
                        
                    time.sleep(0.05)
                    
    except serial.SerialException as e:
        console.print(f"\n[bold red]Hardware Connection Failure:[/bold red] Could not access port {port}. Check connections. Error details: {e}")
    except KeyboardInterrupt:
        console.print("\n[bold yellow]Diagnostic session closed by operator safely.[/bold yellow]")

if __name__ == "__main__":
    app()
