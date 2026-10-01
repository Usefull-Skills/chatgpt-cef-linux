# ChatGPT CEF V2 برای لینوکس

یک کلاینت غیررسمی لینوکسی برای ChatGPT بر پایه Chromium Embedded Framework (CEF) با پوسته بومی، تب‌های native، پشتیبانی RTL فارسی/عربی، Voice/Mic، بازیابی نشست و رابط مدرن مینیمال.

> این پروژه رسمی OpenAI نیست و توسط OpenAI پشتیبانی یا تأیید نشده است. برنامه فقط `https://chatgpt.com/` را با فرآیند ورود عادی ChatGPT نمایش می‌دهد.

## نسخه فعلی

**v0.7.1 — Release Hardening**

این نسخه با build کاملاً تازه، `-Werror`، static analysis، تست چرخه تب، shutdown کنترل‌شده، dependency audit، secret/privacy scan و تست‌های واقعی RTL/LTR، Upload، Clipboard، Voice و single-instance بررسی شده است.

## قابلیت‌های اصلی

- پوسته native مبتنی بر CEF Views
- پنجره frameless و چندتب
- میانبرهای native
- بازیابی atomic نشست
- اجرای single-instance
- محدودسازی مجوزهای Media به origin دقیق ChatGPT
- هدایت لینک‌های خارجی به مرورگر سیستم
- فونت Vazirmatn برای فارسی، بدون قراردادن فایل فونت داخل مخزن
- RTL خودکار برای فارسی و LTR برای انگلیسی/کد/ریاضی
- sandbox فعال؛ استفاده از `--no-sandbox` توصیه نمی‌شود

## ساخت

```bash
./scripts/fetch-cef.sh
./scripts/build-release.sh
```

جزئیات در `docs/BUILDING.md` آمده است.

## داده‌های حساب

پروفایل اجرایی در مسیر زیر قرار می‌گیرد:

```text
~/.config/chatgpt-cef-v2/
```

این پوشه می‌تواند session احراز هویت داشته باشد و نباید در Git یا فایل‌های اشتراکی قرار گیرد.

## برنامه آینده

تمام ایده‌های آینده در `docs/FUTURE_FEATURES.md` طبقه‌بندی شده‌اند و پس از دسترسی Organization باید به Issueهای GitHub تبدیل شوند.
