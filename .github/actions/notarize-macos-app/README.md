# Notarize macOS App Action

This custom GitHub Action handles the complete certificate setup, code signing, and notarization process for macOS applications.

## Usage

```yaml
- name: Code Sign and Notarize App
  uses: ./.github/actions/notarize-macos-app
  with:
    app_path: path/to/MyApp.app
    p12_base64: ${{ secrets.P12_BASE64 }}
    p12_password: ${{ secrets.P12_PASSWORD }}
    apple_id: ${{ secrets.APPLE_ID }}
    apple_id_password: ${{ secrets.APPLE_ID_PASSWORD }}
    team_id: ${{ secrets.APPLE_TEAM_ID }}
    timeout: 30m  # optional, defaults to 15m
```

## Inputs

| Input | Description | Required | Default |
|-------|-------------|----------|---------|
| `app_path` | Path to the .app bundle to notarize | ✅ | |
| `p12_base64` | Base64 encoded P12 certificate file | ✅ | |
| `p12_password` | Password for the P12 certificate | ✅ | |
| `apple_id` | Apple ID for notarization | ✅ | |
| `apple_id_password` | App-specific password for Apple ID | ✅ | |
| `team_id` | Apple Developer Team ID | ✅ | |
| `timeout` | Timeout for notarization (e.g., 15m, 30m) | ❌ | 15m |

## Prerequisites

1. **Apple Developer Account**: Must be enrolled in Apple Developer Program
2. **Developer ID Application Certificate**: Created and exported as P12
3. **App-Specific Password**: Create one at [appleid.apple.com](https://appleid.apple.com)

## Required Secrets

Set these in your GitHub repository settings:

- `P12_BASE64`: Base64 encoded P12 certificate file
- `P12_PASSWORD`: Password for the P12 certificate
- `APPLE_ID`: Your Apple Developer account email
- `APPLE_ID_PASSWORD`: App-specific password (not your regular password)
- `APPLE_TEAM_ID`: Your 10-character Apple Developer Team ID

### Creating P12_BASE64 Secret

```bash
# Convert your P12 file to base64
base64 -i your-certificate.p12 | pbcopy
# Paste the result into GitHub secrets as P12_BASE64
```

## Example Workflow

```yaml
- name: Check out repository
  uses: actions/checkout@v3

- name: Download and Extract App
  # ... your app preparation steps ...

- name: Code Sign and Notarize
  uses: ./.github/actions/notarize-macos-app
  with:
    app_path: path/to/MyApp.app
    p12_base64: ${{ secrets.P12_BASE64 }}
    p12_password: ${{ secrets.P12_PASSWORD }}
    apple_id: ${{ secrets.APPLE_ID }}
    apple_id_password: ${{ secrets.APPLE_ID_PASSWORD }}
    team_id: ${{ secrets.APPLE_TEAM_ID }}
    timeout: 30m
```

## What the Action Does

1. **🔐 Certificate Setup**:
   - Creates temporary keychain
   - Imports P12 certificate
   - Sets keychain permissions
   - Automatically detects certificate identity

2. **✍️ Code Signing**:
   - Deep signs the app with runtime hardening
   - Verifies the signature

3. **📦 Notarization Package**:
   - Creates optimized zip file
   - Reports file size and warns if large

4. **🍎 Apple Submission**:
   - Submits to Apple's notarization service
   - Waits for approval with configurable timeout

5. **📎 Stapling**:
   - Downloads and attaches notarization ticket
   - Enables offline verification

6. **✅ Final Verification**:
   - Confirms app is properly notarized
   - Ready for distribution outside Mac App Store

## Troubleshooting

- **Certificate not found**: Ensure your P12 contains both certificate and private key
- **Notarization timeout**: Increase timeout or check Apple's service status
- **Invalid Team ID**: Verify your Apple Developer Team ID is correct
