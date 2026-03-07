document.addEventListener("DOMContentLoaded", function () {

    const form = document.getElementById("upload-form");
    const message = document.getElementById("message");
    const uploadBtn = document.getElementById("upload-btn");
    const fileInput = document.getElementById("file-input");
    const statsBox = document.getElementById("stats");

    /* ------------------------------
       FORM SUBMISSION
    ------------------------------ */

    form.addEventListener("submit", async function (event) {

        event.preventDefault();

        const formData = new FormData(form);

        try {

            const response = await fetch("/", {
                method: "POST",
                body: formData
            });

            const result = await response.json();

            if (result.success) {

                showMessage("success", result.message);

                /* Show statistics */
                if (result.stats) {
                    statsBox.innerText = JSON.stringify(result.stats, null, 2);
                }

                /* Add download link */
                if (result.download_url) {

                    const downloadBtn = document.createElement("a");
                    downloadBtn.href = result.download_url;
                    downloadBtn.innerText = "⬇ Download File";
                    downloadBtn.className = "submit-btn";
                    downloadBtn.style.display = "block";
                    downloadBtn.style.marginTop = "15px";

                    message.appendChild(document.createElement("br"));
                    message.appendChild(downloadBtn);
                }

            } else {
                showMessage("error", result.error || "Operation failed");
            }

        } catch (error) {
            showMessage("error", error.message);
        }
    });


    /* ------------------------------
       UI EFFECTS
    ------------------------------ */

    function showMessage(type, text) {

        message.innerText = text;
        message.style.color = type === "success" ? "lime" : "red";

        gsap.fromTo(message,
            { opacity: 0, scale: 0.8 },
            { opacity: 1, scale: 1, duration: 0.4 }
        );

        if (type === "error") {
            gsap.to(message, {
                x: -5,
                repeat: 5,
                yoyo: true,
                duration: 0.08
            });
        }
    }


    /* ------------------------------
       FILE UPLOAD DRAG DROP
    ------------------------------ */

    uploadBtn.addEventListener("click", () => fileInput.click());

    uploadBtn.addEventListener("dragover", (event) => {
        event.preventDefault();
        uploadBtn.classList.add("dragging");
    });

    uploadBtn.addEventListener("dragleave", () => {
        uploadBtn.classList.remove("dragging");
    });

    uploadBtn.addEventListener("drop", (event) => {
        event.preventDefault();

        fileInput.files = event.dataTransfer.files;

        if (fileInput.files.length > 0) {
            uploadBtn.innerText = "📂 " + fileInput.files[0].name;
        }
    });


    /* ------------------------------
       THREE.JS BACKGROUND
    ------------------------------ */

    const scene = new THREE.Scene();

    const camera = new THREE.PerspectiveCamera(
        75,
        window.innerWidth / window.innerHeight,
        0.1,
        1000
    );

    const renderer = new THREE.WebGLRenderer({
        canvas: document.getElementById("bg"),
        alpha: true
    });

    renderer.setSize(window.innerWidth, window.innerHeight);
    camera.position.z = 500;

    const starGeometry = new THREE.BufferGeometry();
    const starVertices = [];

    for (let i = 0; i < 600; i++) {
        starVertices.push(
            (Math.random() - 0.5) * 2000,
            (Math.random() - 0.5) * 2000,
            (Math.random() - 0.5) * 2000
        );
    }

    starGeometry.setAttribute(
        "position",
        new THREE.Float32BufferAttribute(starVertices, 3)
    );

    const starMaterial = new THREE.PointsMaterial({
        size: 3,
        blending: THREE.AdditiveBlending
    });

    const stars = new THREE.Points(starGeometry, starMaterial);
    scene.add(stars);

    function animate() {
        requestAnimationFrame(animate);
        stars.rotation.y += 0.002;
        renderer.render(scene, camera);
    }

    animate();

});