import streamlit as st
import hashlib, math, io
from PIL import Image, ImageChops, ImageEnhance, ExifTags
from reportlab.lib.pagesizes import letter
from reportlab.pdfgen import canvas
from reportlab.lib.utils import ImageReader
from datetime import datetime

# ---------- Utility functions ----------
def get_hashes(file_bytes):
    md5 = hashlib.md5(file_bytes).hexdigest()
    sha1 = hashlib.sha1(file_bytes).hexdigest()
    sha256 = hashlib.sha256(file_bytes).hexdigest()
    return {"MD5": md5, "SHA1": sha1, "SHA256": sha256}

def extract_metadata(image):
    info = image.getexif()
    if not info:
        return {"Info": "No EXIF metadata found."}
    metadata = {}
    for tag, val in info.items():
        tag_name = ExifTags.TAGS.get(tag, tag)
        metadata[tag_name] = val
    return metadata

def image_entropy(image):
    gray = image.convert("L")
    hist = gray.histogram()
    total = sum(hist)
    probs = [h / total for h in hist if h != 0]
    return -sum([p * math.log(p, 2) for p in probs])

def ela_analysis(image):
    filename = "temp_ela.jpg"
    image.convert('RGB').save(filename, 'JPEG', quality=90)
    tmp = Image.open(filename)
    diff = ImageChops.difference(image.convert('RGB'), tmp)
    extrema = diff.getextrema()
    max_diff = max(ex[1] for ex in extrema)
    scale = 255.0 / max_diff if max_diff != 0 else 1
    diff = ImageEnhance.Brightness(diff).enhance(scale)
    ela_diff_mean = sum(diff.convert("L").getdata()) / (diff.size[0]*diff.size[1])
    return diff, ela_diff_mean

def calculate_suspicion_level(entropy, ela_mean):
    if entropy < 4.5 and ela_mean < 15:
        return "Low 🟢", "No significant signs of editing detected."
    elif 4.5 <= entropy < 6 or 15 <= ela_mean < 30:
        return "Medium 🟠", "Possible localized editing detected."
    else:
        return "High 🔴", "High likelihood of tampering or image modifications."

def create_pdf_report(details, ela_image, filename="forensic_report.pdf"):
    c = canvas.Canvas(filename, pagesize=letter)
    width, height = letter
    c.setFont("Helvetica-Bold", 16)
    c.drawString(160, height - 50, "DIGITAL IMAGE FORENSIC REPORT")
    c.setFont("Helvetica", 11)
    y = height - 100

    # Header
    c.drawString(50, y, f"Report Generated On: {details['Timestamp']}")
    y -= 25
    c.drawString(50, y, f"File Analyzed: {details['File Name']}")
    y -= 35

    # Hashes
    c.setFont("Helvetica-Bold", 13)
    c.drawString(50, y, "1. File Integrity (Hashes)")
    c.setFont("Helvetica", 11)
    y -= 20
    for k, v in details['Hashes'].items():
        c.drawString(60, y, f"{k}: {v}")
        y -= 15
    y -= 10
    c.drawString(60, y, "Explanation: Hashes are digital fingerprints. Any change in the image changes these values.")
    y -= 30

    # Entropy
    c.setFont("Helvetica-Bold", 13)
    c.drawString(50, y, "2. Image Entropy")
    c.setFont("Helvetica", 11)
    y -= 20
    c.drawString(60, y, f"Entropy Score: {details['Entropy']}")
    y -= 15
    c.drawString(60, y, "Explanation: Measures randomness/complexity. High values may suggest edits.")
    y -= 30

    # Metadata
    c.setFont("Helvetica-Bold", 13)
    c.drawString(50, y, "3. EXIF Metadata")
    c.setFont("Helvetica", 11)
    y -= 20
    if details["Metadata"]:
        for k, v in list(details["Metadata"].items())[:8]:
            c.drawString(60, y, f"{k}: {str(v)[:100]}")
            y -= 15
            if y < 100:
                c.showPage()
                y = height - 100
    else:
        c.drawString(60, y, "No EXIF metadata found (may indicate editing or screenshot).")
        y -= 30

    # Original + ELA
    c.setFont("Helvetica-Bold", 13)
    c.drawString(50, y, "4. Original & ELA Image Comparison")
    y -= 20
    c.setFont("Helvetica", 11)
    c.drawString(60, y, "Left: Original | Right: ELA visualization (bright areas may indicate edits)")
    y -= 20

    # Original and ELA images
    original_reader = ImageReader(details['Original Image'])
    ela_reader = ImageReader(ela_image)
    c.drawImage(original_reader, 60, y-150, width=200, height=150)
    c.drawImage(ela_reader, 280, y-150, width=200, height=150)
    y -= 170

    # Color-coded Suspicion Level
    c.setFont("Helvetica-Bold", 13)
    level_text = details['Suspicion'][0]
    if "Low" in level_text:
        c.setFillColorRGB(0, 0.6, 0)   # Green
    elif "Medium" in level_text:
        c.setFillColorRGB(1, 0.65, 0)  # Orange
    else:
        c.setFillColorRGB(1, 0, 0)     # Red
    c.drawString(50, y, f"Suspicion Level: {level_text}")
    c.setFillColorRGB(0,0,0)
    y -= 20
    c.setFont("Helvetica", 11)
    c.drawString(60, y, f"Interpretation: {details['Suspicion'][1]}")
    y -= 30

    c.setFont("Helvetica-Oblique", 10)
    c.drawString(50, y, "Note: This is an automated analysis for basic forensic evaluation purposes.")
    c.save()
    return filename

# ---------- Streamlit UI ----------
st.set_page_config(page_title="Digital Image Forensic Tool", layout="wide")
st.title("🔍 Digital Image Forensic Analysis Tool")

uploaded_file = st.file_uploader("Upload an image", type=["jpg","jpeg","png"])
if uploaded_file:
    bytes_data = uploaded_file.read()
    img = Image.open(io.BytesIO(bytes_data))
    st.image(img, caption="Uploaded Image", use_container_width=True)

    # Analysis
    st.subheader("Results:")
    hashes = get_hashes(bytes_data)
    entropy_val = round(image_entropy(img),3)
    metadata = extract_metadata(img)
    ela_img, ela_mean = ela_analysis(img)
    suspicion_level = calculate_suspicion_level(entropy_val, ela_mean)

    st.markdown("### 🧾 File Integrity (Hashes)")
    st.json(hashes)

    st.markdown("### 🎚️ Image Entropy")
    st.write(f"Entropy Score: **{entropy_val}** — measures image randomness/complexity.")

    st.markdown("### 📸 EXIF Metadata")
    st.write(metadata)

    st.markdown("### 🩻 Original vs ELA Image")
    col1, col2 = st.columns(2)
    with col1:
        st.image(img, caption="Original Image", use_container_width=True)
    with col2:
        st.image(ela_img, caption="ELA Image (bright areas may indicate edits)", use_container_width=True)

    st.markdown("### ⚠️ Suspicion Level")
    if "Low" in suspicion_level[0]:
        st.markdown(f"<h2 style='color:green'>{suspicion_level[0]}</h2>", unsafe_allow_html=True)
    elif "Medium" in suspicion_level[0]:
        st.markdown(f"<h2 style='color:orange'>{suspicion_level[0]}</h2>", unsafe_allow_html=True)
    else:
        st.markdown(f"<h2 style='color:red'>{suspicion_level[0]}</h2>", unsafe_allow_html=True)
    st.write(suspicion_level[1])

    # Generate PDF report
    report_data = {
        "File Name": uploaded_file.name,
        "Hashes": hashes,
        "Entropy": entropy_val,
        "Metadata": metadata,
        "Suspicion": suspicion_level,
        "Timestamp": str(datetime.now()),
        "Original Image": img
    }
    pdf_path = create_pdf_report(report_data, ela_img)
    with open(pdf_path, "rb") as f:
        st.download_button("📄 Download Forensic Report (PDF with ELA & Suspicion)", f, file_name="forensic_report.pdf")

