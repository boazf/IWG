/*
 * Copyright 2020-2025 Boaz Feldboim
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// SPDX-License-Identifier: Apache-2.0

#include <HTTPServer.h>
#include <HttpContollersUtil.h>
#include <IndexView.h>
#include <SettingsView.h>
#include <DummyView.h>
#include <DefaultView.h>
#include <HistoryView.h>
#include <Filesview.h>
#include <SSEController.h>
#include <FilesController.h>
#include <RecoveryController.h>
#include <SystemController.h>

void InitHttpControllers()
{
    HTTPServer::AddController("/INDEX", IndexView::getInstance);
    HTTPServer::AddController("/SETTINGS", SettingsView::getInstance);
    HTTPServer::AddController("/DUMMY", DummyView::getInstance);
    HTTPServer::AddController("/", DefaultView::getInstance);
    HTTPServer::AddController("/HISTORY", HistoryView::getInstance);
    HTTPServer::AddController("/FILES", FilesView::getInstance);
    HTTPServer::AddController("/API/SSE", SSEController::getInstance);
    HTTPServer::AddController("/API/FILES", FilesController::getInstance);
    HTTPServer::AddController("/API/RECOVERY", RecoveryController::getInstance);
    HTTPServer::AddController("/API/SYSTEM", SystemController::getInstance);
}
