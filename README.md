# Video for Presentation and Prototype --> https://youtu.be/JCAnYKSfNCM

---
# 🛰️ NS-3 LoRaWAN Noise Monitoring Simulation Dashboard

[![NS-3 Compatible](https://img.shields.io/badge/Simulator-NS3--3.44-blue)](https://www.nsnam.org/)
[![LoRaWAN](https://img.shields.io/badge/Module-LoRaWAN-green)](https://gitlab.com/nsnam/lorawan)

This project simulates a **LoRaWAN-based noise monitoring node** using [NS-3](https://www.nsnam.org/) and provides a **real-time dashboard** for:

- Monitoring noise levels
- Viewing device health (battery, temperature, location)
- Manually triggering a status report from the browser

---

## 📦 Requirements

### 🖥️ System Dependencies

Install the following:

```bash
sudo apt update
sudo apt install apache2 php php-sqlite3 sqlite3 g++ cmake git


---

## 🧪 NS-3 & LoRaWAN Setup

This simulation requires:

* ✅ [NS-3 network simulator](https://www.nsnam.org/) — Recommended version: `3.44`
* ✅ [LoRaWAN module](https://gitlab.com/nsnam/lorawan)

### 🔧 Install NS-3

```bash
git clone https://gitlab.com/nsnam/ns-3-dev.git ns-3.44
cd ns-3.44
./ns3 configure --enable-examples --enable-tests
./ns3 build
```

### 📡 Add LoRaWAN Module (if missing)

```bash
cd ns-3.44/src
git clone https://gitlab.com/nsnam/lorawan.git lorawan
cd ..
./ns3 configure
./ns3 build
```

✅ Verify the `lorawan` folder exists in `src/`.

---

## 📁 Project Structure

```
.
├── lora-spl-sim.cc                # NS-3 simulation file
├── simulation_data.db             # Auto-generated SQLite database
├── sound_log.csv                  # Sound log output
├── report.flag                    # File used by dashboard to trigger report
├── index.html                     # Dashboard UI (refreshes every 2 seconds)
├── data.php                       # API endpoint returning JSON data
├── trigger-report.php            # PHP script to write report.flag
├── docs/
│   └── screenshot-dashboard.png   # Dashboard screenshot (optional)
```

---

## ▶️ Running the Simulation

1. **Place the `.cc` file inside your NS-3 scratch directory**:

```bash
cp lora-spl-sim.cc ~/ns-3.44/scratch/
```

2. **Run the simulation**:

```bash
cd ~/ns-3.44
./ns3 run scratch/lora-spl-sim
```

> This starts logging sound levels and sending status reports automatically.

---

## 🌐 Web Dashboard Setup

1. **Move the following files to your web server**:

```bash
sudo cp index.html data.php trigger-report.php /var/www/html/
```

2. **Ensure PHP script writes the flag file to the same path NS-3 is reading from**. Example inside `trigger-report.php`:

```php
$flagPath = '/home/gabriel/Desktop/Noise Monitoring Prototype/ns3-simulation/ns-3.44/report.flag';
file_put_contents($flagPath, 'REPORT');
```

3. **Give write permissions** to the directory for testing:

```bash
sudo chmod 777 "/home/gabriel/Desktop/Noise Monitoring Prototype/ns3-simulation/ns-3.44"
```

4. **Open the dashboard in your browser**:

```
http://localhost/index.html
```

---

## 💡 Dashboard Features

* ✅ **Live noise updates** from the last packet in SQLite
* ✅ **Live device status** (voltage, current, temp, GPS)
* ✅ **Color-coded noise levels**:

  * Green: ≤ 65 dB
  * Amber: 66–85 dB
  * Red: > 85 dB (includes alert + exclamation)
* ✅ **Device name shown** (parsed from table name)
* ✅ **Button to trigger status report** from the node

---

## 📤 GitHub Pages (Optional)

You can serve the dashboard statically using GitHub Pages, but note that:

* PHP won’t run on GitHub Pages.
* Use [Apache](https://httpd.apache.org/) or [Nginx + PHP-FPM](https://www.nginx.com/resources/wiki/start/topics/examples/phpfastcgionnginx/) locally.

---

## 🛠️ Troubleshooting

### 🟥 Button not working / `report.flag` missing?

* Make sure `trigger-report.php` has write access to the correct path.
* Try using `/tmp/report.flag` instead.
* Check:

  ```bash
  tail /var/log/apache2/error.log
  ```

### 🟥 Simulation crashes?

Check for messages like:

```
PHY was in TX mode when attempting to close a receive window
```

Try increasing delay between status and sound sends, or reduce TX frequency jitter.

---

## 🧑‍💻 Contributing

Pull requests welcome! If you'd like to:

* Add multi-node support
* Switch to SQLite-based command triggers
* Extend dashboard charts

Feel free to fork and contribute.

---

## 📜 License

This project is released under the MIT License.
Use freely for educational and prototyping purposes.

---

## 🙋 Need Help?

Open an issue or reach out in the [NS-3 user forum](https://groups.google.com/g/ns-3-users).

```

---

📌 You can now:

1. Create a file named `README.md` in your GitHub repo root.
2. Paste the full Markdown above.
3. (Optional) Add `docs/screenshot-dashboard.png` to show off the UI.

Let me know if you'd like me to generate a `Makefile`, GitHub Actions workflow, or Dockerfile for full automation.
```
