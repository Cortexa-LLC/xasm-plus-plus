#!/bin/bash
# Upgrade SonarQube Community Edition to Developer Edition
# All data (projects, settings, history) is preserved via Docker volumes

set -e

echo "🔄 Upgrading SonarQube to Developer Edition..."
echo ""
echo "This will:"
echo "  1. Stop Community Edition containers"
echo "  2. Start Developer Edition containers (same volumes)"
echo "  3. Preserve all data (projects, settings, history)"
echo ""
read -p "Continue? (y/n) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "❌ Cancelled"
    exit 1
fi

echo ""
echo "📦 Pulling Developer Edition image..."
docker pull sonarqube:10-developer

echo ""
echo "🛑 Stopping Community Edition..."
cd .ai-pack
docker-compose -f docker-compose.sonarqube.yml down

echo ""
echo "🚀 Starting Developer Edition..."
cd ..
docker-compose \
  -f .ai-pack/docker-compose.sonarqube.yml \
  -f docker-compose.sonarqube-dev.yml \
  up -d

echo ""
echo "⏳ Waiting for SonarQube to start (30 seconds)..."
sleep 30

echo ""
echo "✅ Developer Edition is now running!"
echo ""
echo "📋 Next steps:"
echo ""
echo "1. Apply your license key:"
echo "   • Open: http://localhost:9000"
echo "   • Login: admin/admin"
echo "   • Go to: Administration → Configuration → License Manager"
echo "   • Paste your license key and click Save"
echo ""
echo "2. Verify C++ plugin installed:"
echo "   • Go to: Administration → Marketplace → Installed"
echo "   • Check for 'C/C++ (CFamily)' plugin"
echo "   • If missing, install from Marketplace and restart"
echo ""
echo "3. Run analysis:"
echo "   source sonar.env && sonar-scanner"
echo ""
echo "🎉 Done!"
