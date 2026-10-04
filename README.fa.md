# Remote Commander Browser

**Remote Commander Browser** یک پوستهٔ دسکتاپ native و چندسکویی برای ChatGPT بر پایهٔ Chromium Embedded Framework (CEF) است. برنامه تب‌های native، پشتیبانی RTL فارسی/عربی، Voice/Mic، بازیابی نشست و پنل فقط‌خواندنی Remote Commander را فراهم می‌کند.

> این پروژه رسمی OpenAI نیست و توسط OpenAI پشتیبانی یا تأیید نشده است. برنامه `https://chatgpt.com/` را با فرایند ورود عادی ChatGPT باز می‌کند.

## نسخه فعلی

**v0.8.0-rc.3 — کاندید پیش‌نمایش Windows/Linux / هنوز Stable نیست**

مسیرهای build و lifecycle ویندوز و لینوکس جداگانه qualify شده‌اند. نسخهٔ پایدار `v0.7.1` تا تکمیل CI دقیق rc.3، تست واقعی UI، اتصال Commander↔Browser، گفت‌وگوی آزمایشی ایزوله، بازیابی طولانی‌مدت و rollback نصب، مرجع بازگشت باقی می‌ماند.

## قابلیت‌های اصلی

- پوسته native مبتنی بر CEF Views برای Windows و Linux/X11
- پنجره frameless و چندتب
- میانبرهای native و بازیابی atomic نشست
- single-instance
- محدودسازی مجوزهای Media به origin دقیق ChatGPT
- هدایت لینک‌های خارجی به مرورگر سیستم
- Vazirmatn-first و RTL/LTR خودکار
- پنل فقط‌خواندنی Remote Commander
- محافظ ACL/فایل خصوصی native در ویندوز
- sandbox و بسته‌بندی release محدود و قابل‌ممیزی

## سازگاری و حفظ نشست

نام محصول و artifactها به **Remote Commander Browser** تغییر می‌کند، اما در rc.3 شناسه‌های داخلی قدیمی عمداً حفظ می‌شوند:

```text
chatgpt-cef-v2
~/.config/chatgpt-cef-v2/
```

این تصمیم برای جلوگیری از جابه‌جایی یا از دست رفتن session احراز هویت است. تغییر نام محصول به معنی کپی یا استخراج cookie/credential نیست.

## ساخت

Linux:

```bash
./scripts/fetch-cef.sh
./scripts/build-release.sh
./scripts/static-qa.sh
./scripts/runtime-self-test.sh
```

Windows:

```powershell
./scripts/fetch-cef-windows.ps1
./scripts/build-windows.ps1
./scripts/package-windows.ps1
```

## مخزن

نام حرفه‌ای هدف برای مخزن: **`remote-commander-browser`**. مخزن فعلی تا پذیرش دقیق rc.3 و مهاجرت بدون شکستن PR/CI/remotes مرجع باقی می‌ماند.

جزئیات معماری و شواهد در `docs/ARCHITECTURE.md`، `docs/PROJECT_BRAIN.md` و `docs/PROJECT_KNOWLEDGE_EVIDENCE.md` ثبت می‌شود.
