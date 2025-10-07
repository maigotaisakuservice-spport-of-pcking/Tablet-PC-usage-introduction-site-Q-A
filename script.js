// AI Creator Dashboard Scripts (v2 - with persisted login and corrected scopes)

// --- Configuration ---
const CLIENT_ID = "556927001491-l2b8t4hoqllj3np998t4l49agt02tvfv.apps.googleusercontent.com";
const GAS_URL = "https://script.google.com/macros/s/AKfycbz9l_Ys_6C5TM-HXIy_mGu0zJKUMrwDO2kVrw_h7rFM9-fCWaFHpr5GXSHDmWoJyi5frw/exec";
// Corrected scopes for full data access including comments and user profile
const SCOPES = 'https://www.googleapis.com/auth/youtube.force-ssl https://www.googleapis.com/auth/yt-analytics.readonly https://www.googleapis.com/auth/userinfo.profile';

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
            checkForStoredToken();
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
        if (tokenClient) tokenClient.requestAccessToken();
        else alert('Google認証の準備ができていません。');
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

    const applyRpmBtn = document.getElementById('apply-rpm-btn');
    if(applyRpmBtn) applyRpmBtn.addEventListener('click', () => {
        const newRpm = parseFloat(document.getElementById('manual-rpm-input').value);
        if (!isNaN(newRpm) && newRpm >= 0) {
            currentRpm = newRpm;
            isRpmAiSet = false;
            updateRevenueUI();
        } else {
            alert('有効なRPM値を入力してください。');
        }
    });

    const resetRpmBtn = document.getElementById('reset-rpm-btn');
    if(resetRpmBtn) resetRpmBtn.addEventListener('click', () => {
        isRpmAiSet = true;
        updateRevenueUI();
    });

    const videoListContainer = document.getElementById('video-list-container');
    if (videoListContainer) videoListContainer.addEventListener('click', (e) => {
        const target = e.target;
        if (target.closest('.btn-delete')) handleVideoDelete(target.closest('tr').dataset.videoId);
        if (target.closest('.btn-edit')) handleVideoEdit(target.closest('tr'));
        if (target.closest('.btn-save')) handleVideoSave(target.closest('tr'));
        if (target.closest('.btn-cancel')) handleCancelEdit(target.closest('tr'));
    });

    const analyticsContainer = document.getElementById('analytics-content');
    if (analyticsContainer) analyticsContainer.addEventListener('click', (e) => {
        if (e.target.closest('.btn-show-more')) handleShowMore(e.target.closest('.analytics-card'));
    });

    const commentListContainer = document.getElementById('comment-list-container');
    if (commentListContainer) commentListContainer.addEventListener('click', (e) => {
        const target = e.target;
        if (target.closest('.btn-generate-reply')) handleGenerateReply(target);
        if (target.closest('.ai-reply-suggestion')) {
            navigator.clipboard.writeText(target.textContent).then(() => alert('コピーしました！'), () => alert('コピーに失敗しました。'));
        }
    });

    const generateIdeasBtn = document.getElementById('generate-ideas-btn');
    if (generateIdeasBtn) generateIdeasBtn.addEventListener('click', handleGenerateIdeas);

    // --- Core Functions ---

    function escapeHTML(str) {
        const p = document.createElement('p');
        p.textContent = str;
        return p.innerHTML;
    }

    function checkForStoredToken() {
        const storedToken = localStorage.getItem('youtubeDashboardToken');
        const storedExpiry = localStorage.getItem('youtubeDashboardTokenExpiry');
        if (storedToken && storedExpiry && new Date().getTime() < parseInt(storedExpiry, 10)) {
            accessToken = storedToken;
            fetchUserProfile().then(profile => {
                showDashboard(profile.name);
                fetchAllData();
                dataUpdateInterval = setInterval(fetchAllData, 3600 * 1000);
            }).catch(() => handleLogout());
        } else {
            handleLogout();
        }
    }

    async function handleTokenResponse(tokenResponse) {
        if (tokenResponse.error) return alert(`認証に失敗しました: ${tokenResponse.error_description || '不明なエラー'}`);
        accessToken = tokenResponse.access_token;
        const expiryTime = new Date().getTime() + (tokenResponse.expires_in * 1000);
        localStorage.setItem('youtubeDashboardToken', accessToken);
        localStorage.setItem('youtubeDashboardTokenExpiry', expiryTime);
        try {
            const profile = await fetchUserProfile();
            showDashboard(profile.name);
            fetchAllData();
            dataUpdateInterval = setInterval(fetchAllData, 3600 * 1000);
        } catch (error) {
            alert("ユーザー情報の取得または初期データの読み込みに失敗しました。");
            handleLogout();
        }
    }

    async function fetchUserProfile() {
        const response = await fetch('https://www.googleapis.com/oauth2/v3/userinfo', { headers: { 'Authorization': `Bearer ${accessToken}` } });
        if (!response.ok) throw new Error('Failed to fetch user profile');
        return await response.json();
    }

    function handleLogout() {
        if (accessToken) google.accounts.oauth2.revoke(accessToken, () => {});
        localStorage.removeItem('youtubeDashboardToken');
        localStorage.removeItem('youtubeDashboardTokenExpiry');
        accessToken = null;
        clearInterval(dataUpdateInterval);
        showLoginScreen();
    }

    function fetchAllData() {
        fetchAndRenderVideos();
        fetchAndRenderRevenueData();
        fetchAndRenderAnalytics();
        fetchAndRenderComments();
    }

    async function fetchDataFromGAS(prompt, data) {
        const response = await fetch(GAS_URL, { method: 'POST', mode: 'cors', body: JSON.stringify({ prompt, data }) });
        if (!response.ok) throw new Error('GAS request failed');
        return await response.json();
    }

    // --- Video Management Functions ---

    async function fetchAndRenderVideos() {
        if (!accessToken) return;
        const container = document.getElementById('video-list-container');
        container.innerHTML = '<p class="loading-message">動画リストを読み込み中...</p>';
        try {
            const channelResponse = await fetch('https://www.googleapis.com/youtube/v3/channels?part=contentDetails&mine=true', { headers: { 'Authorization': `Bearer ${accessToken}` } });
            const channelData = await channelResponse.json();
            const uploadsPlaylistId = channelData.items[0].contentDetails.relatedPlaylists.uploads;
            const playlistResponse = await fetch(`https://www.googleapis.com/youtube/v3/playlistItems?part=snippet,status&playlistId=${uploadsPlaylistId}&maxResults=50`, { headers: { 'Authorization': `Bearer ${accessToken}` } });
            const playlistData = await playlistResponse.json();
            renderVideoList(playlistData.items);
        } catch (error) {
            container.innerHTML = '<p class="error-message">動画リストの読み込みに失敗しました。</p>';
        }
    }

    function renderVideoList(videos) {
        const container = document.getElementById('video-list-container');
        if (!videos || videos.length === 0) return container.innerHTML = '<p>動画はありません。</p>';
        const table = document.createElement('table');
        table.className = 'video-table';
        table.innerHTML = `<thead><tr><th>動画</th><th>公開状況</th><th>投稿日</th><th>操作</th></tr></thead><tbody>
            ${videos.map(v => {
                const s = v.snippet;
                const id = s.resourceId.videoId;
                return `<tr data-video-id="${id}"><td class="video-info"><a href="https://youtube.com/watch?v=${id}" target="_blank"><img src="${s.thumbnails.default.url}" class="video-thumbnail"></a><div><a href="https://youtube.com/watch?v=${id}" target="_blank" class="video-title">${escapeHTML(s.title)}</a><p class="video-description">${escapeHTML(s.description.substring(0,100))}...</p></div></td><td>${v.status.privacyStatus}</td><td>${new Date(s.publishedAt).toLocaleDateString()}</td><td class="video-actions"><button class="btn-edit">編集</button><button class="btn-delete">削除</button></td></tr>`;
            }).join('')}</tbody>`;
        container.innerHTML = '';
        container.appendChild(table);
    }

    function handleVideoEdit(row) {
        if(row.classList.contains('editing')) return;
        row.classList.add('editing');
        row.dataset.original = row.innerHTML;
        const title = row.querySelector('.video-title').textContent;
        const desc = row.querySelector('.video-description').textContent.replace('...','');
        const status = row.querySelector('td:nth-child(2)').textContent;
        row.querySelector('.video-info').innerHTML = `<div class="edit-fields"><input type="text" class="edit-title" value="${escapeHTML(title)}"><textarea class="edit-description">${escapeHTML(desc)}</textarea></div>`;
        row.querySelector('td:nth-child(2)').innerHTML = `<select class="edit-status"><option value="public" ${status==='public'?'selected':''}>public</option><option value="private" ${status==='private'?'selected':''}>private</option><option value="unlisted" ${status==='unlisted'?'selected':''}>unlisted</option></select>`;
        row.querySelector('.video-actions').innerHTML = `<button class="btn-save">保存</button><button class="btn-cancel">ｷｬﾝｾﾙ</button>`;
    }

    function handleCancelEdit(row) {
        if(!row.classList.contains('editing')) return;
        row.innerHTML = row.dataset.original;
        row.classList.remove('editing');
    }

    async function handleVideoSave(row) {
        const id = row.dataset.videoId;
        const newTitle = row.querySelector('.edit-title').value;
        const newDesc = row.querySelector('.edit-description').value;
        const newStatus = row.querySelector('.edit-status').value;
        try {
            const videoRes = await fetch(`https://www.googleapis.com/youtube/v3/videos?part=snippet&id=${id}`, { headers: { 'Authorization': `Bearer ${accessToken}` } });
            const videoData = await videoRes.json();
            const payload = { id, snippet: { title: newTitle, description: newDesc, categoryId: videoData.items[0].snippet.categoryId }, status: { privacyStatus: newStatus } };
            const updateRes = await fetch('https://www.googleapis.com/youtube/v3/videos?part=snippet,status', { method: 'PUT', headers: { 'Authorization': `Bearer ${accessToken}`, 'Content-Type': 'application/json' }, body: JSON.stringify(payload) });
            if(updateRes.ok) {
                alert('更新しました。');
                fetchAndRenderVideos();
            } else {
                const err = await updateRes.json();
                throw new Error(err.error.message);
            }
        } catch (e) {
            alert(`更新に失敗しました: ${e.message}`);
        }
    }

    async function handleVideoDelete(id) {
        if(!confirm(`この動画を削除しますか？\n元に戻せません。`)) return;
        try {
            const res = await fetch(`https://www.googleapis.com/youtube/v3/videos?id=${id}`, { method: 'DELETE', headers: { 'Authorization': `Bearer ${accessToken}` } });
            if(res.status === 204) {
                alert('削除しました。');
                document.querySelector(`tr[data-video-id="${id}"]`)?.remove();
            } else {
                const err = await res.json();
                throw new Error(err.error.message);
            }
        } catch (e) {
            alert(`削除に失敗しました: ${e.message}`);
        }
    }

    // --- Revenue Tab Functions ---

    async function fetchAndRenderRevenueData() {
        if (!accessToken) return;
        const end = new Date(), start = new Date();
        start.setDate(end.getDate() - 28);
        const format = d => d.toISOString().split('T')[0];
        const range = `startDate=${format(start)}&endDate=${format(end)}`;
        try {
            const res = await fetch(`https://youtubeanalytics.googleapis.com/v2/reports?ids=channel==MINE&${range}&metrics=views,estimatedRevenue`, { headers: { 'Authorization': `Bearer ${accessToken}` } });
            const data = await res.json();
            if (data.rows && data.rows.length > 0) {
                const [views, revenueUSD] = data.rows[0];
                lastRevenueData = { views, revenue: revenueUSD * 150, hasActual: true, actualRpm: views > 0 ? (revenueUSD * 150 / views) * 1000 : 0 };
            } else {
                await fetchViewsForRevenueEstimation(range);
            }
        } catch (e) {
            await fetchViewsForRevenueEstimation(range);
        } finally {
            updateRevenueUI();
        }
    }

    async function fetchViewsForRevenueEstimation(range) {
        try {
            const res = await fetch(`https://youtubeanalytics.googleapis.com/v2/reports?ids=channel==MINE&${range}&metrics=views`, { headers: { 'Authorization': `Bearer ${accessToken}` } });
            const data = await res.json();
            lastRevenueData = { views: (data.rows && data.rows[0]) ? data.rows[0][0] : 0, revenue: 0, hasActual: false, actualRpm: 0 };
        } catch(e) {
            lastRevenueData = { views: 0, revenue: 0, hasActual: false, actualRpm: 0 };
        }
    }

    function updateRevenueUI() {
        if (!lastRevenueData) return;
        const { views, revenue, hasActual, actualRpm } = lastRevenueData;
        let finalRpm = currentRpm, finalRevenue = (views / 1000) * currentRpm;
        let revenueType = '(推定)', rpmType = '(手動)';
        if (isRpmAiSet) {
            if (hasActual) {
                finalRpm = actualRpm;
                finalRevenue = revenue;
                revenueType = '(実績)';
                rpmType = '(実績)';
            } else {
                finalRpm = 500;
                finalRevenue = (views / 1000) * finalRpm;
                rpmType = '(AI)';
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
        document.querySelectorAll('.ranking-list').forEach(l => l.innerHTML = '<li>読み込み中...</li>');
        document.getElementById('ai-summary-content').textContent = 'AI分析を生成中...';
        const end = new Date(), start = new Date();
        start.setDate(end.getDate() - 28);
        const format = d => d.toISOString().split('T')[0];
        const range = `startDate=${format(start)}&endDate=${format(end)}`;
        try {
            const res = await fetch(`https://youtubeanalytics.googleapis.com/v2/reports?ids=channel==MINE&${range}&metrics=views,estimatedMinutesWatched,likes,comments&dimensions=video&sort=-views`, { headers: { 'Authorization': `Bearer ${accessToken}` } });
            const data = await res.json();
            if (!data.rows || data.rows.length === 0) throw new Error('No analytics data');
            const videoIds = data.rows.map(r => r[0]);
            const videosRes = await fetch(`https://www.googleapis.com/youtube/v3/videos?part=snippet&id=${videoIds.join(',')}`, { headers: { 'Authorization': `Bearer ${accessToken}` } });
            const videosData = await videosRes.json();
            const titles = {};
            videosData.items.forEach(i => titles[i.id] = i.snippet.title);
            const combined = data.rows.map(r => ({ id: r[0], title: titles[r[0]] || r[0], views: r[1], minutes: r[2], likes: r[3], comments: r[4] }));
            renderRankingList(combined, 'views-ranking-card', 'views', '回');
            renderRankingList(combined, 'watch-time-ranking-card', 'minutes', '分');
            renderRankingList(combined, 'likes-ranking-card', 'likes', '件');
            renderRankingList(combined, 'comments-ranking-card', 'comments', '件');
            fetchAiSummary(combined);
        } catch (e) {
            document.getElementById('ai-summary-content').textContent = 'アナリティクスデータの読込に失敗しました。';
        }
    }

    function renderRankingList(data, cardId, metric, unit) {
        const card = document.getElementById(cardId);
        const list = card.querySelector('.ranking-list');
        const sorted = [...data].sort((a, b) => b[metric] - a[metric]);
        card.dataset.fullData = JSON.stringify(sorted);
        card.dataset.metric = metric;
        card.dataset.unit = unit;
        const top5 = sorted.slice(0, 5);
        list.innerHTML = top5.map(item => `<li><a href="https://youtube.com/watch?v=${item.id}" target="_blank" class="video-title" title="${escapeHTML(item.title)}">${escapeHTML(item.title)}</a><span class="metric-value">${item[metric].toLocaleString()} ${unit}</span></li>`).join('');
        const btn = card.querySelector('.btn-show-more');
        if (sorted.length > 5) {
            btn.style.display = 'block';
            btn.textContent = 'もっと見る';
            card.dataset.expanded = 'false';
        } else {
            btn.style.display = 'none';
        }
    }

    function handleShowMore(card) {
        const expanded = card.dataset.expanded === 'true';
        const data = JSON.parse(card.dataset.fullData);
        const metric = card.dataset.metric;
        const unit = card.dataset.unit;
        const list = card.querySelector('.ranking-list');
        const btn = card.querySelector('.btn-show-more');
        const toRender = expanded ? data.slice(0, 5) : data;
        list.innerHTML = toRender.map(item => `<li><a href="https://youtube.com/watch?v=${item.id}" target="_blank" class="video-title" title="${escapeHTML(item.title)}">${escapeHTML(item.title)}</a><span class="metric-value">${item[metric].toLocaleString()} ${unit}</span></li>`).join('');
        card.dataset.expanded = !expanded;
        btn.textContent = expanded ? 'もっと見る' : '閉じる';
    }

    async function fetchAiSummary(analyticsData) {
        const summaryEl = document.getElementById('ai-summary-content');
        try {
            const top5 = analyticsData.slice(0, 5);
            const promptData = { topPerformers: top5.map(v => ({ title: v.title, views: v.views, minutes: v.minutes, likes: v.likes })) };
            const prompt = `以下のYouTubeアナリティクスデータを分析し、プロのコンサルタントとして、チャンネルの強み、弱み、具体的な改善点を3つの箇条書きで要約してください:\n${JSON.stringify(promptData)}`;
            const res = await fetchDataFromGAS(prompt, {});
            if (res.status === 'success' && res.data.analysis) {
                summaryEl.textContent = res.data.analysis;
            } else {
                throw new Error(res.message || 'AI応答の解析失敗');
            }
        } catch (e) {
            summaryEl.textContent = 'AI総評の生成に失敗しました。';
        }
    }

    // --- Comment Reply Tab Functions ---

    async function fetchAndRenderComments() {
        if (!accessToken) return;
        const container = document.getElementById('comment-list-container');
        container.innerHTML = '<p class="loading-message">コメントを読み込み中...</p>';
        try {
            const channelRes = await fetch('https://www.googleapis.com/youtube/v3/channels?part=id&mine=true', { headers: { 'Authorization': `Bearer ${accessToken}` } });
            const channelData = await channelRes.json();
            const channelId = channelData.items[0].id;
            const res = await fetch(`https://www.googleapis.com/youtube/v3/commentThreads?part=snippet&allThreadsRelatedToChannelId=${channelId}&order=time&maxResults=20`, { headers: { 'Authorization': `Bearer ${accessToken}` } });
            const data = await res.json();
            renderCommentList(data.items);
        } catch (e) {
            container.innerHTML = '<p class="error-message">コメントの読込に失敗しました。</p>';
        }
    }

    function renderCommentList(comments) {
        const container = document.getElementById('comment-list-container');
        if (!comments || comments.length === 0) return container.innerHTML = '<p>コメントはありません。</p>';
        container.innerHTML = comments.map(item => {
            const c = item.snippet.topLevelComment.snippet;
            return `<div class="comment-item"><div class="comment-header"><img src="${c.authorProfileImageUrl}" class="comment-author-avatar"><span class="comment-author-name">${escapeHTML(c.authorDisplayName)}</span></div><p class="comment-body">${escapeHTML(c.textDisplay)}</p><div class="comment-actions"><button class="btn-generate-reply" data-comment-text="${escapeHTML(c.textOriginal)}">AIで返信</button></div><div class="ai-reply-container" style="display:none;"></div></div>`;
        }).join('');
    }

    async function handleGenerateReply(btn) {
        const text = btn.dataset.commentText;
        const container = btn.closest('.comment-item').querySelector('.ai-reply-container');
        container.style.display = 'block';
        container.innerHTML = '<h4>AIが返信を生成中...</h4>';
        const prompt = `視聴者コメントに対し、丁寧で感謝が伝わる返信文を3パターン考えてください。:\nコメント: "${text}"\n返信案:`;
        try {
            const res = await fetchDataFromGAS(prompt, {});
            if (res.status === 'success' && res.data.analysis) {
                const suggestions = res.data.analysis.split('\n').map(s => s.trim().replace(/^- /,'')).filter(Boolean);
                container.innerHTML = `<h4>AI返信候補 (クリックでコピー):</h4>${suggestions.map(s => `<div class="ai-reply-suggestion">${escapeHTML(s)}</div>`).join('')}`;
            } else {
                throw new Error(res.message || 'AI応答の解析失敗');
            }
        } catch (e) {
            container.innerHTML = '<h4>返信の生成に失敗しました。</h4>';
        }
    }

    // --- Creation Support Tab Functions ---

    async function handleGenerateIdeas() {
        const input = document.getElementById('video-idea-input');
        const container = document.getElementById('ai-ideas-container');
        const content = document.getElementById('ai-ideas-content');
        const theme = input.value.trim();
        if (!theme) return alert('テーマを入力してください。');
        container.style.display = 'block';
        content.textContent = 'AIがアイデアを生成中...';
        const prompt = `あなたはプロのYouTubeコンサルタントです。テーマ「${theme}」について、クリックしたくなるタイトル5つと、具体的な動画構成案（導入,本編,まとめ）を提案してください。`;
        try {
            const res = await fetchDataFromGAS(prompt, {});
            if (res.status === 'success' && res.data.analysis) {
                content.textContent = res.data.analysis;
            } else {
                throw new Error(res.message || 'AI応答の解析失敗');
            }
        } catch (e) {
            content.textContent = 'アイデアの生成に失敗しました。';
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