# Third-party notices

## Chromium Embedded Framework (CEF)

This project is built against:

- CEF: `154.0.32+g682c378+chromium-154.0.8037.58`
- CEF commit: `682c378d70d5780061e96644dca16ddd8fd157a9`
- Chromium: `154.0.8037.58`
- Chromium commit: `a654841425914cbb703a2931e07b70a83aedbafd`

CEF is BSD licensed. A copy of the CEF license is included at `third_party/CEF_LICENSE.txt`.

The CEF binary distribution also contains software under additional licenses. Binary packages produced by this project must include the CEF `LICENSE.txt` and `CREDITS.html` from the exact CEF distribution used to build the package.

## Vazirmatn

Vazirmatn is an optional system font used for Persian/Arabic UI rendering. Font files are **not distributed by this repository**. Install it from your Linux distribution (Ubuntu package: `fonts-vazirmatn`) and follow the font project's own license terms.

## OpenAI / ChatGPT

This repository contains no OpenAI SDK, model weights, API credentials, or proprietary OpenAI source code. It loads the public ChatGPT web application in an embedded Chromium surface after normal user authentication.
