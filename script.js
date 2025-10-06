// AI Creator Dashboard Scripts

// --- Configuration ---
const CLIENT_ID = "556927001491-l2b8t4hoqllj3np998t4l49agt02tvfv.apps.googleusercontent.com";
const GAS_URL = "https://script.google.com/macros/s/AKfycbz9l_Ys_6C5TM-HXIy_mGu0zJKUMrwDO2kVrw_h7rFM9-fCWaFHpr5GXSHDmWoJyi5frw/exec";
const SCOPES = 'https://www.googleapis.com/auth/youtube https://www.googleapis.com/auth/yt-analytics.readonly';

// --- State Management ---
let tokenClient;
let accessToken = null;
let dataUpdateInterval = null;
let currentRpm = 500; // Default RPM
let isRpmAiSet = true; // True if using actual/AI RPM, false if manual
let lastRevenueData = null; // Cache for recalculations

document.addEventListener('DOMContentLoaded', () => {
    // --- UI Elements ---
    const loginScreen = document.getElementById('login-screen');
    const dashboardScreen = document.getElementById('dashboard-screen');
    const googleLoginBtn = document.getElementById('google-login-btn');
    const logoutBtn = document.getElementById('logout-btn');
    const tabNav = document.querySelector('.tab-nav');
    const tabContents = document.querySelectorAll('.tab-content');
    const tabLinks = document.querySelectorAll('.tab-link');
    const userNameDisplay = document.getElementById('user-name-display');

    // --- Google Auth Initialization ---
    const checkGoogle = setInterval(() => {
        if (typeof google !== 'undefined' && google.accounts) {
            clearInterval(checkGoogle);
            initializeGsi();
        }
    }, 100);

    function initializeGsi() {
        tokenClient = google.accounts.oauth2.initTokenClient({
            client_id: CLIENT_ID,
            scope: SCOPES,
            callback: handleTokenResponse,
        });
    }

    // --- Event Listeners ---
    googleLoginBtn.addEventListener('click', () => {
        if (tokenClient) {
            tokenClient.requestAccessToken();
        } else {
            alert('Google認証の準備ができていません。少し待ってからもう一度お試しください。');
        }
    });

    logoutBtn.addEventListener('click', handleLogout);

    if (tabNav) {
        tabNav.addEventListener('click', (e) => {
            const clickedTab = e.target.closest('.tab-link');
            if (!clickedTab) return;
            tabLinks.forEach(link => link.classList.remove('active'));
            tabContents.forEach(content => content.classList.remove('active'));
            clickedTab.classList.add('active');
            document.getElementById(clickedTab.dataset.tab)?.classList.add('active');
        });
    }

    // --- Revenue Management Event Listeners ---
    const applyRpmBtn = document.getElementById('apply-rpm-btn');
    const resetRpmBtn = document.getElementById('reset-rpm-btn');
    const manualRpmInput = document.getElementById('manual-rpm-input');

    if (applyRpmBtn) {
        applyRpmBtn.addEventListener('click', () => {
            const newRpm = parseFloat(manualRpmInput.value);
            if (!isNaN(newRpm) && newRpm >= 0) {
                currentRpm = newRpm;
                isRpmAiSet = false;
                updateRevenueUI(); // Recalculate with new manual RPM
            } else {
                alert('有効なRPM値を入力してください。');
            }
        });
    }

    if (resetRpmBtn) {
        resetRpmBtn.addEventListener('click', () => {
            isRpmAiSet = true;
            // This will revert to the last known actual RPM or the default AI one.
            updateRevenueUI();
        });
    }

    // --- Video Management Event Delegation ---
    const videoListContainer = document.getElementById('video-list-container');
    if (videoListContainer) {
        videoListContainer.addEventListener('click', (e) => {
            const deleteButton = e.target.closest('.btn-delete');
            if (deleteButton) handleVideoDelete(deleteButton.dataset.videoId);

            const editButton = e.target.closest('.btn-edit');
            if (editButton) handleVideoEdit(editButton.closest('tr'));

            const saveButton = e.target.closest('.btn-save');
            if(saveButton) handleVideoSave(saveButton.closest('tr'));

            const cancelButton = e.target.closest('.btn-cancel');
            if(cancelButton) handleCancelEdit(cancelButton.closest('tr'));
        });
    }

    // --- Analytics Tab Event Delegation ---
    const analyticsContainer = document.getElementById('analytics-content');
    if (analyticsContainer) {
        analyticsContainer.addEventListener('click', (e) => {
            const showMoreButton = e.target.closest('.btn-show-more');
            if (showMoreButton) {
                handleShowMore(showMoreButton.closest('.analytics-card'));
            }
        });
    }

    // --- Comment Reply Tab Event Delegation ---
    const commentListContainer = document.getElementById('comment-list-container');
    if (commentListContainer) {
        commentListContainer.addEventListener('click', (e) => {
            const generateButton = e.target.closest('.btn-generate-reply');
            if (generateButton) {
                handleGenerateReply(generateButton);
            }

            const suggestion = e.target.closest('.ai-reply-suggestion');
            if (suggestion) {
                navigator.clipboard.writeText(suggestion.textContent).then(() => {
                    alert('返信文をコピーしました！');
                }).catch(err => {
                    console.error('Could not copy text: ', err);
                    alert('コピーに失敗しました。');
                });
            }
        });
    }

    // --- Creation Support Tab Event Listener ---
    const generateIdeasBtn = document.getElementById('generate-ideas-btn');
    if (generateIdeasBtn) {
        generateIdeasBtn.addEventListener('click', handleGenerateIdeas);
    }

    // --- Core Functions ---

    function escapeHTML(str) {
        const p = document.createElement('p');
        p.textContent = str;
        return p.innerHTML;
    }

    async function handleTokenResponse(tokenResponse) {
        if (tokenResponse.error) {
            console.error("Authentication error:", tokenResponse.error);
            alert(`認証に失敗しました: ${tokenResponse.error_description || '不明なエラーです。'}`);
            return;
        }
        accessToken = tokenResponse.access_token;

        try {
            const profile = await fetchUserProfile();
            showDashboard(profile.name);
            fetchAllData();
            if (dataUpdateInterval) clearInterval(dataUpdateInterval);
            dataUpdateInterval = setInterval(fetchAllData, 3600 * 1000);
        } catch (error) {
            console.error("Failed to fetch user profile or initial data:", error);
            alert("ユーザー情報の取得または初期データの読み込みに失敗しました。");
            handleLogout();
        }
    }

    async function fetchUserProfile() {
        const response = await fetch('https://www.googleapis.com/oauth2/v3/userinfo', {
            headers: { 'Authorization': `Bearer ${accessToken}` }
        });
        if (!response.ok) throw new Error(`Failed to fetch user profile. Status: ${response.status}`);
        return await response.json();
    }

    function handleLogout() {
        if (accessToken) {
            google.accounts.oauth2.revoke(accessToken, () => console.log('Access token revoked.'));
            accessToken = null;
        }
        if (dataUpdateInterval) clearInterval(dataUpdateInterval);
        showLoginScreen();
    }

    function fetchAllData() {
        console.log("Fetching all data at", new Date().toLocaleTimeString());
        fetchAndRenderVideos();
        fetchAndRenderRevenueData();
        fetchAndRenderAnalytics();
        fetchAndRenderComments();
        // The AI summary call is now triggered from within fetchAndRenderAnalytics
    }

    async function fetchDataFromGAS(prompt, data) {
        const response = await fetch(GAS_URL, {
            method: 'POST',
            mode: 'cors',
            body: JSON.stringify({ prompt, data })
        });
        if (!response.ok) throw new Error(`GAS request failed with status ${response.status}`);
        return await response.json();
    }

    // --- Video Management Functions ---

    async function fetchAndRenderVideos() {
        if (!accessToken) return;
        const container = document.getElementById('video-list-container');
        container.innerHTML = '<p class="loading-message">動画リストを読み込んでいます...</p>';
        try {
            const channelResponse = await fetch('https://www.googleapis.com/youtube/v3/channels?part=contentDetails&mine=true', {
                headers: { 'Authorization': `Bearer ${accessToken}` }
            });
            if (!channelResponse.ok) throw new Error(`YouTube API error (channels): ${channelResponse.status}`);
            const channelData = await channelResponse.json();
            const uploadsPlaylistId = channelData.items[0].contentDetails.relatedPlaylists.uploads;

            const playlistResponse = await fetch(`https://www.googleapis.com/youtube/v3/playlistItems?part=snippet,status&playlistId=${uploadsPlaylistId}&maxResults=50`, {
                headers: { 'Authorization': `Bearer ${accessToken}` }
            });
            if (!playlistResponse.ok) throw new Error(`YouTube API error (playlistItems): ${playlistResponse.status}`);
            const playlistData = await playlistResponse.json();
            renderVideoList(playlistData.items);
        } catch (error) {
            console.error("Error fetching video list:", error);
            container.innerHTML = '<p class="error-message">動画リストの読み込みに失敗しました。</p>';
        }
    }

    function renderVideoList(videos) {
        const container = document.getElementById('video-list-container');
        if (!videos || videos.length === 0) {
            container.innerHTML = '<p>アップロードされた動画はありません。</p>';
            return;
        }
        const table = document.createElement('table');
        table.className = 'video-table';
        table.innerHTML = `
            <thead><tr><th>動画</th><th>公開状況</th><th>投稿日</th><th>操作</th></tr></thead>
            <tbody>
                ${videos.map(video => {
                    const s = video.snippet;
                    const id = s.resourceId.videoId;
                    return `
                        <tr data-video-id="${id}">
                            <td class="video-info">
                                <a href="https://www.youtube.com/watch?v=${id}" target="_blank"><img src="${s.thumbnails.default.url}" alt="" class="video-thumbnail"></a>
                                <div class="video-title-desc">
                                    <a href="https://www.youtube.com/watch?v=${id}" target="_blank" class="video-title">${escapeHTML(s.title)}</a>
                                    <p class="video-description">${escapeHTML((s.description || '').substring(0, 100))}...</p>
                                </div>
                            </td>
                            <td>${video.status.privacyStatus}</td>
                            <td>${new Date(s.publishedAt).toLocaleDateString()}</td>
                            <td class="video-actions">
                                <button class="btn-edit" data-video-id="${id}">編集</button>
                                <button class="btn-delete" data-video-id="${id}">削除</button>
                            </td>
                        </tr>`;
                }).join('')}
            </tbody>`;
        container.innerHTML = '';
        container.appendChild(table);
    }

    function handleVideoEdit(videoRow) {
        if (videoRow.classList.contains('editing')) return;
        videoRow.classList.add('editing');
        videoRow.dataset.originalHtml = videoRow.innerHTML;

        const infoCell = videoRow.querySelector('.video-info');
        const statusCell = videoRow.querySelector('td:nth-child(2)');
        const actionsCell = videoRow.querySelector('.video-actions');

        const title = infoCell.querySelector('.video-title').textContent;
        const description = infoCell.querySelector('.video-description').textContent.replace('...', '').trim();
        const status = statusCell.textContent.trim();

        infoCell.innerHTML = `<div class="edit-fields"><input type="text" class="edit-title" value="${escapeHTML(title)}"><textarea class="edit-description">${escapeHTML(description)}</textarea></div>`;
        statusCell.innerHTML = `<select class="edit-status"><option value="public" ${status==='public'?'selected':''}>public</option><option value="private" ${status==='private'?'selected':''}>private</option><option value="unlisted" ${status==='unlisted'?'selected':''}>unlisted</option></select>`;
        actionsCell.innerHTML = `<button class="btn-save">保存</button><button class="btn-cancel">キャンセル</button>`;
    }

    function handleCancelEdit(videoRow) {
        if (!videoRow.classList.contains('editing')) return;
        videoRow.innerHTML = videoRow.dataset.originalHtml;
        videoRow.classList.remove('editing');
        delete videoRow.dataset.originalHtml;
    }

    async function handleVideoSave(videoRow) {
        const videoId = videoRow.dataset.videoId;
        if (!accessToken) return alert("認証情報がありません。");

        const newTitle = videoRow.querySelector('.edit-title').value;
        const newDescription = videoRow.querySelector('.edit-description').value;
        const newStatus = videoRow.querySelector('.edit-status').value;

        try {
            const videoGetResponse = await fetch(`https://www.googleapis.com/youtube/v3/videos?part=snippet&id=${videoId}`, { headers: { 'Authorization': `Bearer ${accessToken}` } });
            if (!videoGetResponse.ok) throw new Error('Failed to fetch current video data for update.');
            const videoGetData = await videoGetResponse.json();
            const categoryId = videoGetData.items[0].snippet.categoryId;

            const updatePayload = { id: videoId, snippet: { title: newTitle, description: newDescription, categoryId: categoryId }, status: { privacyStatus: newStatus } };

            const videoUpdateResponse = await fetch('https://www.googleapis.com/youtube/v3/videos?part=snippet,status', {
                method: 'PUT',
                headers: { 'Authorization': `Bearer ${accessToken}`, 'Content-Type': 'application/json' },
                body: JSON.stringify(updatePayload)
            });

            if (videoUpdateResponse.ok) {
                alert('動画情報が正常に更新されました。');
                fetchAndRenderVideos();
            } else {
                const errorData = await videoUpdateResponse.json();
                throw new Error(errorData.error.message);
            }
        } catch (error) {
            console.error('Error during video update:', error);
            alert(`動画の更新中にエラーが発生しました: ${error.message}`);
        }
    }

    async function handleVideoDelete(videoId) {
        if (!accessToken) return alert("認証情報がありません。");
        if (!confirm(`本当にこの動画を削除しますか？\nこの操作は元に戻せません。\nVideo ID: ${videoId}`)) return;

        try {
            const response = await fetch(`https://www.googleapis.com/youtube/v3/videos?id=${videoId}`, {
                method: 'DELETE',
                headers: { 'Authorization': `Bearer ${accessToken}` }
            });
            if (response.status === 204) {
                alert('動画が正常に削除されました。');
                document.querySelector(`tr[data-video-id="${videoId}"]`)?.remove();
            } else {
                const errorData = await response.json();
                throw new Error(errorData.error.message);
            }
        } catch (error) {
            console.error('Error during video deletion:', error);
            alert(`動画の削除中にエラーが発生しました: ${error.message}`);
        }
    }

    // --- Revenue Tab Functions ---

    async function fetchAndRenderRevenueData() {
        if (!accessToken) return;
        const endDate = new Date();
        const startDate = new Date();
        startDate.setDate(endDate.getDate() - 28);
        const format = (d) => d.toISOString().split('T')[0];
        const dateRange = `startDate=${format(startDate)}&endDate=${format(endDate)}`;
        const metrics = 'views,estimatedRevenue';
        const url = `https://youtubeanalytics.googleapis.com/v2/reports?ids=channel==MINE&${dateRange}&metrics=${metrics}`;

        try {
            const response = await fetch(url, { headers: { 'Authorization': `Bearer ${accessToken}` } });
            if (!response.ok) throw new Error(`Analytics API error: ${response.status}`);
            const data = await response.json();

            if (data.columnHeaders.length > 1 && data.rows && data.rows.length > 0) {
                const totalViews = data.rows[0][0];
                const totalRevenueUSD = data.rows[0][1];
                const JPY_RATE = 150; // Placeholder exchange rate
                lastRevenueData = {
                    views: totalViews,
                    revenue: totalRevenueUSD * JPY_RATE,
                    hasActual: true,
                    actualRpm: totalViews > 0 ? (totalRevenueUSD * JPY_RATE / totalViews) * 1000 : 0
                };
            } else {
                await fetchViewsForRevenueEstimation(dateRange);
            }
        } catch (error) {
            console.error("Error fetching revenue data, falling back to views-only:", error);
            await fetchViewsForRevenueEstimation(dateRange);
        } finally {
            updateRevenueUI();
        }
    }

    async function fetchViewsForRevenueEstimation(dateRange) {
        const url = `https://youtubeanalytics.googleapis.com/v2/reports?ids=channel==MINE&${dateRange}&metrics=views`;
        try {
            const response = await fetch(url, { headers: { 'Authorization': `Bearer ${accessToken}` } });
            if (!response.ok) throw new Error(`Analytics API error (views only): ${response.status}`);
            const data = await response.json();
            const totalViews = (data.rows && data.rows.length > 0) ? data.rows[0][0] : 0;
            lastRevenueData = { views: totalViews, revenue: 0, hasActual: false, actualRpm: 0 };
        } catch (error) {
            console.error("Could not fetch views data:", error);
            lastRevenueData = { views: 0, revenue: 0, hasActual: false, actualRpm: 0 };
        }
    }

    function updateRevenueUI() {
        if (!lastRevenueData) return;
        const { views, revenue, hasActual, actualRpm } = lastRevenueData;

        let finalRpm = currentRpm;
        let finalRevenue = (views / 1000) * currentRpm;
        let revenueType = '(推定)';
        let rpmType = '(手動設定)';

        if (isRpmAiSet) {
            if (hasActual) {
                finalRpm = actualRpm;
                finalRevenue = revenue;
                revenueType = '(実績)';
                rpmType = '(実績RPM)';
            } else {
                finalRpm = 500;
                finalRevenue = (views / 1000) * finalRpm;
                rpmType = '(AI/デフォルト)';
            }
            currentRpm = finalRpm;
        }

        document.getElementById('estimated-revenue').textContent = `¥${Math.round(finalRevenue).toLocaleString()}`;
        document.getElementById('revenue-views').textContent = views.toLocaleString();
        document.getElementById('revenue-rpm').textContent = `¥${Math.round(finalRpm).toLocaleString()}`;
        document.getElementById('revenue-type-label').textContent = revenueType;
        document.getElementById('rpm-type-label').textContent = rpmType;
        document.getElementById('manual-rpm-input').value = Math.round(currentRpm);
    }

    // --- Analytics Tab Functions ---

    async function fetchAndRenderAnalytics() {
        if (!accessToken) return;

        // Show loading state
        document.querySelectorAll('.ranking-list').forEach(list => { list.innerHTML = '<li>読み込み中...</li>'; });
        document.getElementById('ai-summary-content').textContent = 'AIによる分析を生成しています...';

        const endDate = new Date();
        const startDate = new Date();
        startDate.setDate(endDate.getDate() - 28);
        const format = (d) => d.toISOString().split('T')[0];
        const dateRange = `startDate=${format(startDate)}&endDate=${format(endDate)}`;
        const metrics = 'views,estimatedMinutesWatched,likes,comments';
        const dimensions = 'video';
        const url = `https://youtubeanalytics.googleapis.com/v2/reports?ids=channel==MINE&${dateRange}&metrics=${metrics}&dimensions=${dimensions}&sort=-views`;

        try {
            const response = await fetch(url, { headers: { 'Authorization': `Bearer ${accessToken}` } });
            if (!response.ok) throw new Error(`Analytics API error: ${response.status}`);
            const data = await response.json();

            if (!data.rows || data.rows.length === 0) {
                document.getElementById('analytics-content').innerHTML += '<p>十分なアナリティクスデータがありません。</p>';
                return;
            }

            const videoIds = data.rows.map(row => row[0]);
            const videosResponse = await fetch(`https://www.googleapis.com/youtube/v3/videos?part=snippet&id=${videoIds.join(',')}`, {
                headers: { 'Authorization': `Bearer ${accessToken}` }
            });
            if (!videosResponse.ok) throw new Error(`Video titles API error: ${videosResponse.status}`);
            const videosData = await videosResponse.json();

            const videoTitles = {};
            videosData.items.forEach(item => { videoTitles[item.id] = item.snippet.title; });

            const combinedData = data.rows.map(row => ({
                videoId: row[0],
                title: videoTitles[row[0]] || row[0],
                views: row[1],
                minutes: row[2],
                likes: row[3],
                comments: row[4]
            }));

            renderRankingList(combinedData, 'views-ranking-card', 'views', '回');
            renderRankingList(combinedData, 'watch-time-ranking-card', 'minutes', '分');
            renderRankingList(combinedData, 'likes-ranking-card', 'likes', '件');
            renderRankingList(combinedData, 'comments-ranking-card', 'comments', '件');

            fetchAiSummary(combinedData);

        } catch (error) {
            console.error("Error fetching analytics data:", error);
            document.getElementById('ai-summary-content').textContent = 'アナリティクスデータの読み込みに失敗しました。';
        }
    }

    function renderRankingList(data, cardId, metric, unit) {
        const cardElement = document.getElementById(cardId);
        const container = cardElement.querySelector('.ranking-list');
        const sortedData = [...data].sort((a, b) => b[metric] - a[metric]);

        cardElement.dataset.fullRankingData = JSON.stringify(sortedData);
        cardElement.dataset.metric = metric;
        cardElement.dataset.unit = unit;

        const top5 = sortedData.slice(0, 5);

        container.innerHTML = top5.map(item => `
            <li>
                <a href="https://www.youtube.com/watch?v=${item.videoId}" target="_blank" class="video-title" title="${escapeHTML(item.title)}">${escapeHTML(item.title)}</a>
                <span class="metric-value">${item[metric].toLocaleString()} ${unit}</span>
            </li>
        `).join('');

        const showMoreBtn = cardElement.querySelector('.btn-show-more');
        if (sortedData.length > 5) {
            showMoreBtn.style.display = 'block';
            showMoreBtn.textContent = 'もっと見る';
            cardElement.dataset.expanded = 'false';
        } else {
            showMoreBtn.style.display = 'none';
        }
    }

    function handleShowMore(cardElement) {
        const isExpanded = cardElement.dataset.expanded === 'true';
        const fullData = JSON.parse(cardElement.dataset.fullRankingData);
        const metric = cardElement.dataset.metric;
        const unit = cardElement.dataset.unit;
        const listContainer = cardElement.querySelector('.ranking-list');
        const button = cardElement.querySelector('.btn-show-more');

        const itemsToRender = isExpanded ? fullData.slice(0, 5) : fullData;

        listContainer.innerHTML = itemsToRender.map(item => `
            <li>
                <a href="https://www.youtube.com/watch?v=${item.videoId}" target="_blank" class="video-title" title="${escapeHTML(item.title)}">${escapeHTML(item.title)}</a>
                <span class="metric-value">${item[metric].toLocaleString()} ${unit}</span>
            </li>
        `).join('');

        cardElement.dataset.expanded = !isExpanded;
        button.textContent = isExpanded ? 'もっと見る' : '閉じる';
    }

    async function fetchAiSummary(analyticsData) {
        const summaryElement = document.getElementById('ai-summary-content');
        try {
            const top5Views = analyticsData.sort((a,b) => b.views - a.views).slice(0, 5);
            const dataForPrompt = {
                totalVideos: analyticsData.length,
                topPerformers: top5Views.map(v => ({ title: v.title, views: v.views, minutes: v.minutes, likes: v.likes })),
            };
            const prompt = `以下のYouTubeチャンネルのアナリティクスデータを分析し、チャンネル全体の強み、弱み、そして具体的な改善点をプロのコンサルタントのように3つの箇条書きで要約してください。:\n${JSON.stringify(dataForPrompt, null, 2)}`;

            const response = await fetchDataFromGAS(prompt, {});

            if (response.status === 'success' && response.data.analysis) {
                summaryElement.textContent = response.data.analysis;
            } else {
                throw new Error(response.message || 'AIからの応答を解析できませんでした。');
            }
        } catch (error) {
            console.error("Error fetching AI summary:", error);
            summaryElement.textContent = 'AIによる総評の生成に失敗しました。';
        }
    }

    // --- Comment Reply Tab Functions ---

    async function fetchAndRenderComments() {
        if (!accessToken) return;
        const container = document.getElementById('comment-list-container');
        container.innerHTML = '<p class="loading-message">最新のコメントを読み込んでいます...</p>';

        try {
            // Fetch the most recent top-level comments on the channel
            const url = `https://www.googleapis.com/youtube/v3/commentThreads?part=snippet&allThreadsRelatedToChannelId={CHANNEL_ID}&order=time&maxResults=20`;
            // Note: The API requires a channel ID. We need to fetch it first.
            const channelResponse = await fetch('https://www.googleapis.com/youtube/v3/channels?part=id&mine=true', {
                headers: { 'Authorization': `Bearer ${accessToken}` }
            });
            if (!channelResponse.ok) throw new Error(`YouTube API error (channelId for comments): ${channelResponse.status}`);
            const channelData = await channelResponse.json();
            const channelId = channelData.items[0].id;

            const commentsUrl = `https://www.googleapis.com/youtube/v3/commentThreads?part=snippet&allThreadsRelatedToChannelId=${channelId}&order=time&maxResults=20`;
            const response = await fetch(commentsUrl, { headers: { 'Authorization': `Bearer ${accessToken}` } });
            if (!response.ok) throw new Error(`Comments API error: ${response.status}`);
            const data = await response.json();

            renderCommentList(data.items);
        } catch (error) {
            console.error("Error fetching comments:", error);
            container.innerHTML = '<p class="error-message">コメントの読み込みに失敗しました。</p>';
        }
    }

    function renderCommentList(comments) {
        const container = document.getElementById('comment-list-container');
        if (!comments || comments.length === 0) {
            container.innerHTML = '<p>表示するコメントはありません。</p>';
            return;
        }
        container.innerHTML = comments.map(item => {
            const comment = item.snippet.topLevelComment.snippet;
            return `
                <div class="comment-item" data-comment-id="${item.id}">
                    <div class="comment-header">
                        <img src="${comment.authorProfileImageUrl}" alt="avatar" class="comment-author-avatar">
                        <span class="comment-author-name">${escapeHTML(comment.authorDisplayName)}</span>
                    </div>
                    <p class="comment-body">${escapeHTML(comment.textDisplay)}</p>
                    <div class="comment-actions">
                        <button class="btn-generate-reply" data-comment-text="${escapeHTML(comment.textOriginal)}">AIで返信文を生成</button>
                    </div>
                    <div class="ai-reply-container" style="display: none;"></div>
                </div>
            `;
        }).join('');
    }

    async function handleGenerateReply(button) {
        const commentText = button.dataset.commentText;
        const commentItem = button.closest('.comment-item');
        const replyContainer = commentItem.querySelector('.ai-reply-container');

        replyContainer.style.display = 'block';
        replyContainer.innerHTML = '<h4>AIが返信を生成中...</h4>';

        const prompt = `以下の視聴者からのコメントに対して、チャンネル運営者として丁寧で、かつ感謝の気持ちが伝わるような返信文を3パターン考えてください。元のコメントの文脈を考慮し、ポジティブな雰囲気で返信してください。\n\n---\nコメント: "${commentText}"\n---\n返信文案:`;

        try {
            const response = await fetchDataFromGAS(prompt, {});
            if (response.status === 'success' && response.data.analysis) {
                // Assuming the AI returns suggestions separated by newlines
                const suggestions = response.data.analysis.split('\n').map(s => s.trim().replace(/^- /,'')).filter(s => s);
                replyContainer.innerHTML = `
                    <h4>AIによる返信文候補 (クリックしてコピー):</h4>
                    ${suggestions.map(s => `<div class="ai-reply-suggestion">${escapeHTML(s)}</div>`).join('')}
                `;
            } else {
                throw new Error(response.message || 'AIからの応答が不正です。');
            }
        } catch (error) {
            console.error("Error generating AI reply:", error);
            replyContainer.innerHTML = '<h4>返信の生成に失敗しました。</h4>';
        }
    }

    // --- Creation Support Tab Functions ---

    async function handleGenerateIdeas() {
        const ideaInput = document.getElementById('video-idea-input');
        const ideasContainer = document.getElementById('ai-ideas-container');
        const ideasContent = document.getElementById('ai-ideas-content');
        const ideaText = ideaInput.value.trim();

        if (!ideaText) {
            alert('動画のテーマやキーワードを入力してください。');
            return;
        }

        ideasContainer.style.display = 'block';
        ideasContent.textContent = 'AIがアイデアを生成中です...';

        const prompt = `あなたはプロのYouTubeコンサルタントです。以下のテーマについて、視聴者の興味を引くような魅力的な動画のアイデアを提案してください。提案には、以下の要素を必ず含めてください。\n1. クリックしたくなるような動画タイトル案を5つ\n2. 視聴者を飽きさせないための具体的な動画構成案（導入、本編、まとめ）\n\n---\nテーマ: "${ideaText}"\n---\n\n提案:`;

        try {
            const response = await fetchDataFromGAS(prompt, {});
            if (response.status === 'success' && response.data.analysis) {
                ideasContent.textContent = response.data.analysis;
            } else {
                throw new Error(response.message || 'AIからの応答が不正です。');
            }
        } catch (error) {
            console.error("Error generating AI ideas:", error);
            ideasContent.textContent = 'アイデアの生成に失敗しました。';
        }
    }

    // --- UI State Changers ---

    function showDashboard(userName) {
        userNameDisplay.textContent = `ようこそ、${userName}さん`;
        loginScreen.style.display = 'none';
        dashboardScreen.style.display = 'flex';
        document.body.style.justifyContent = 'flex-start';
        document.body.style.alignItems = 'flex-start';
        document.body.style.textAlign = 'left';
    }

    function showLoginScreen() {
        userNameDisplay.textContent = 'ようこそ、ゲストさん';
        dashboardScreen.style.display = 'none';
        loginScreen.style.display = 'block';
        document.body.style.justifyContent = 'center';
        document.body.style.alignItems = 'center';
        document.body.style.textAlign = 'center';
    }
});