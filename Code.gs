// Google Apps Script Backend for AI Creator Dashboard

// --- Configuration ---
const ALLOWED_REFERER = "https://anm.service.search-up.f5.si/";
const DEV_REFERER = "http://localhost:8080/";

/**
 * Main entry point for all frontend POST requests.
 */
function doPost(e) {
  // --- Security Check ---
  const referer = e.headers.referer;
  if (!referer || (!referer.startsWith(ALLOWED_REFERER) && !referer.startsWith(DEV_REFERER))) {
    Logger.log("Forbidden request from referer: " + referer);
    return createJsonResponse({ status: 'error', message: 'Forbidden' });
  }

  // --- Request Processing ---
  try {
    const requestData = JSON.parse(e.postData.contents);
    const { prompt, data } = requestData;

    if (!prompt) {
      return createJsonResponse({ status: 'error', message: 'Bad Request: "prompt" is missing.' });
    }

    // --- AI Processing ---
    const aiResponse = callGeminiApi(prompt, data);

    return createJsonResponse({ status: 'success', data: aiResponse });

  } catch (error) {
    Logger.log("Error in doPost: " + error.toString());
    return createJsonResponse({ status: 'error', message: 'Internal Server Error' });
  }
}

/**
 * Calls the real Gemini API to get an analysis.
 * @param {string} prompt The prompt to send to the AI.
 * @param {object} data The data to be analyzed by the AI (not used in this version).
 * @returns {object} An object containing the AI's analysis.
 */
function callGeminiApi(prompt, data) {
  try {
    const API_KEY = PropertiesService.getScriptProperties().getProperty('GEMINI_API_KEY');
    if (!API_KEY) {
      throw new Error("Gemini API key is not set in Script Properties.");
    }

    const API_URL = "https://generativelanguage.googleapis.com/v1beta/models/gemini-pro:generateContent?key=" + API_KEY;

    const payload = {
      "contents": [{
        "parts": [{
          "text": prompt
        }]
      }]
    };

    const options = {
      'method': 'post',
      'contentType': 'application/json',
      'payload': JSON.stringify(payload),
      'muteHttpExceptions': true // Prevents throwing an exception for non-2xx responses
    };

    const response = UrlFetchApp.fetch(API_URL, options);
    const responseData = JSON.parse(response.getContentText());

    if (response.getResponseCode() >= 400) {
        Logger.log("Gemini API Error Response: " + response.getContentText());
        throw new Error(responseData.error.message || "Gemini API returned an error.");
    }

    if (responseData.candidates && responseData.candidates[0].content.parts[0].text) {
      const aiText = responseData.candidates[0].content.parts[0].text;
      return {
        analysis: aiText,
        suggestions: []
      };
    } else {
      Logger.log("Unexpected Gemini API response structure: " + response.getContentText());
      throw new Error("Could not parse a valid response from the AI.");
    }

  } catch (error) {
    Logger.log("Error in callGeminiApi: " + error.toString());
    return {
      analysis: "AIによる分析中にエラーが発生しました。詳細はサーバーログを確認してください。",
      error: error.toString()
    };
  }
}


/**
 * Creates a standard JSON response object for the web app.
 */
function createJsonResponse(payload) {
  const jsonString = JSON.stringify(payload);
  return ContentService.createTextOutput(jsonString)
    .setMimeType(ContentService.MimeType.JSON);
}

/**
 * Handles CORS preflight (OPTIONS) requests from the browser.
 */
function doOptions(e) {
  const allowedOrigin = "https://anm.service.search-up.f5.si";
  const devOrigin = "http://localhost:8080";
  const requestOrigin = e.headers.origin;

  let originToAllow = null;
  if (requestOrigin === allowedOrigin) {
    originToAllow = allowedOrigin;
  } else if (requestOrigin === devOrigin) {
    originToAllow = devOrigin;
  }

  if (originToAllow) {
    return ContentService.createTextOutput()
      .setHeader('Access-Control-Allow-Origin', originToAllow)
      .setHeader('Access-Control-Allow-Methods', 'POST, OPTIONS')
      .setHeader('Access-control-allow-headers', 'Content-Type');
  }

  return ContentService.createTextOutput();
}