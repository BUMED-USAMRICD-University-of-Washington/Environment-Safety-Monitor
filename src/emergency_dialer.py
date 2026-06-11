import os
import json
from twilio.rest import Client

# -------------------------------------------------------------------------
# EMERGENCY DISPATCH CONFIGURATION (Read via environment variables)
# -------------------------------------------------------------------------
TWILIO_ACCOUNT_SID = os.environ.get("TWILIO_ACCOUNT_SID", "AC_your_account_sid")
TWILIO_AUTH_TOKEN  = os.environ.get("TWILIO_AUTH_TOKEN", "your_auth_token")
TWILIO_PHONE_NUM   = os.environ.get("TWILIO_PHONE_NUM", "+15550001111")
EMERGENCY_DISPATCH = os.environ.get("EMERGENCY_DISPATCH_NUM", "+1911") # Typically routes to a standard 10-digit PSAP line during testing

def initiate_emergency_audio_report(facility_address, room_name, room_volume_m3, current_o2):
    """
    Calculates displaced nitrogen, generates a standardized text-to-speech script,
    and initiates an automated emergency broadcast call over an outbound telephony trunk.
    """
    # 1. Calculate approximate nitrogen gas release volume
    # Under standard atmospheric baselines, dropping below 21.0% O2 reveals the leakage curve
    o2_fraction_dropped = 1.0 - (current_o2 / 21.0)
    estimated_nitrogen_released_m3 = max(0.0, room_volume_m3 * o2_fraction_dropped)

    # 2. Formulate the explicit emergency dispatch vocal script
    # The message repeats to ensure automated clarity for human public safety dispatchers
    alert_script = (
        f"Automated life safety warning. Automated life safety warning. "
        f"An environmental hazard has been detected at {facility_address}. "
        f"Location baseline, room {room_name}. "
        f"Critical hypoxia threshold breached. Calculated oxygen level is {current_o2:.1f} percent. "
        f"Estimated asphyxiant gas displacement calculation indicates approximately "
        f"{estimated_nitrogen_released_m3:.1f} cubic meters of nitrogen gas have been released. "
        f"Immediate hazardous materials and emergency medical response is required. "
        f"Evacuation protocols have been initiated at the site. "
        f"Message will repeat. "
        f"Hazard location, {facility_address}, room {room_name}. "
        f"Oxygen is at {current_o2:.1f} percent. "
        f"Nitrogen release calculation is {estimated_nitrogen_released_m3:.1f} cubic meters."
    )

    # 3. Construct Twilio Voice TwiML instruction block
    # This instructs the telephony server to convert the generated text string to spoken audio
    twiml_payload = (
        f"<Response>"
        f"  <Say voice='alice' language='en-US'>{alert_script}</Say>"
        f"</Response>"
    )

    try:
        # Initialize outbound REST API carrier client
        client = Client(TWILIO_ACCOUNT_SID, TWILIO_AUTH_TOKEN)

        print(f"[ALERT] Initiating outbound emergency call loop to {EMERGENCY_DISPATCH}...")
        call = client.calls.create(
            to=EMERGENCY_DISPATCH,
            from_=TWILIO_PHONE_NUM,
            twiml=twiml_payload
        )
        print(f"[SUCCESS] Call successfully routed to trunk line. SID Reference: {call.sid}")
        return True

    except Exception as e:
        print(f"[FATAL LOG] Failed to dispatch outbound emergency voice packet: {str(e)}")
        return False

# -------------------------------------------------------------------------
# LOCAL INTEGRATION HOOK (Simulating an incoming critical GPU event)
# -------------------------------------------------------------------------
if __name__ == "__main__":
    # Example Parameters parsed from an active server anomaly event
    test_facility = "Building 4, One Hundred Technology Way, Newark New Jersey"
    test_room = "Cryogenic Containment Vault B"
    vault_volume = 250.0  # 250 cubic meters volume space
    measured_o2 = 14.2    # Critical hypoxia state (Breaches the 18% relay trip point)

    # Execute dispatch pipeline
    initiate_emergency_audio_report(test_facility, test_room, vault_volume, measured_o2)
