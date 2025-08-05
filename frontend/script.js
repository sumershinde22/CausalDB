// CausalDB Frontend JavaScript
class CausalDBFrontend {
  constructor() {
    this.events = [];
    this.apiBase = "http://localhost:8080/api";
    this.init();
  }

  init() {
    this.bindEvents();
    this.loadEvents();
    this.setupModal();
  }

  bindEvents() {
    // Form submissions
    document.getElementById("addEventForm").addEventListener("submit", (e) => {
      e.preventDefault();
      this.addEvent();
    });

    document.getElementById("searchForm").addEventListener("submit", (e) => {
      e.preventDefault();
      this.searchEvent();
    });

    // Action buttons
    document.getElementById("refreshBtn").addEventListener("click", () => {
      this.loadEvents();
    });

    document.getElementById("exportBtn").addEventListener("click", () => {
      this.exportData();
    });
  }

  setupModal() {
    const modal = document.getElementById("eventModal");
    const closeBtn = document.querySelector(".close");

    closeBtn.onclick = () => {
      modal.style.display = "none";
    };

    window.onclick = (event) => {
      if (event.target === modal) {
        modal.style.display = "none";
      }
    };
  }

  async callBackendAPI(endpoint, data = null) {
    const options = {
      method: data ? "POST" : "GET",
      headers: {
        "Content-Type": "application/json",
      },
    };

    if (data) {
      options.body = JSON.stringify(data);
    }

    const response = await fetch(`${this.apiBase}/${endpoint}`, options);

    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }

    return await response.json();
  }

  async addEvent() {
    const form = document.getElementById("addEventForm");
    const formData = new FormData(form);

    const eventId = parseInt(formData.get("eventId"));
    const eventData = formData.get("eventData");
    const parentIdsStr = formData.get("parentIds");

    // Parse parent IDs
    const parentIds = parentIdsStr.trim()
      ? parentIdsStr
          .split(" ")
          .map((id) => parseInt(id.trim()))
          .filter((id) => !isNaN(id))
      : [];

    const event = {
      id: eventId,
      data: eventData,
      parent_count: parentIds.length,
      parents: parentIds,
    };

    try {
      await this.callBackendAPI("events", event);

      this.showMessage("Event added successfully!", "success");
      form.reset();
      this.loadEvents();
    } catch (error) {
      this.showMessage("Failed to add event: " + error.message, "error");
    }
  }

  async searchEvent() {
    const form = document.getElementById("searchForm");
    const formData = new FormData(form);
    const searchId = parseInt(formData.get("searchId"));

    try {
      // For now, we'll search in the loaded events
      // In a real implementation, you'd have a specific endpoint for searching
      const event = this.events.find((e) => e.id === searchId);
      if (event) {
        this.showEventModal(event);
      } else {
        this.showMessage("Event not found.", "error");
      }
    } catch (error) {
      this.showMessage("Failed to search event: " + error.message, "error");
    }
  }

  async loadEvents() {
    try {
      this.events = await this.callBackendAPI("events");
      this.renderEvents();
      this.updateStats();
      this.renderGraph();
    } catch (error) {
      this.showMessage("Failed to load events: " + error.message, "error");
      // Fallback to sample data if server is not running
      this.events = this.getSampleData();
      this.renderEvents();
      this.updateStats();
      this.renderGraph();
    }
  }

  getSampleData() {
    return [
      {
        id: 1,
        data: "Project initialization",
        parent_count: 0,
        parents: [],
      },
      {
        id: 2,
        data: "Database schema design",
        parent_count: 1,
        parents: [1],
      },
      {
        id: 3,
        data: "API endpoint implementation",
        parent_count: 2,
        parents: [1, 2],
      },
      {
        id: 4,
        data: "Frontend development",
        parent_count: 1,
        parents: [3],
      },
      {
        id: 5,
        data: "Testing and debugging",
        parent_count: 2,
        parents: [3, 4],
      },
    ];
  }

  renderEvents() {
    const eventsList = document.getElementById("eventsList");
    eventsList.innerHTML = "";

    if (this.events.length === 0) {
      eventsList.innerHTML = `
                <div class="event-card" style="text-align: center; color: #6b7280; font-style: italic;">
                    <i class="fas fa-inbox" style="font-size: 2rem; margin-bottom: 10px;"></i>
                    <p>No events found. Add your first event using the form on the left.</p>
                </div>
            `;
      return;
    }

    this.events.forEach((event) => {
      const eventCard = this.createEventCard(event);
      eventsList.appendChild(eventCard);
    });
  }

  createEventCard(event) {
    const card = document.createElement("div");
    card.className = "event-card";
    card.onclick = () => this.showEventModal(event);

    const parentTags =
      event.parents && event.parents.length > 0
        ? event.parents
            .map((parentId) => `<span class="parent-tag">${parentId}</span>`)
            .join("")
        : '<span style="color: #9ca3af; font-style: italic;">No parents</span>';

    card.innerHTML = `
            <div class="event-header">
                <span class="event-id">#${event.id}</span>
            </div>
            <div class="event-data">${this.escapeHtml(event.data)}</div>
            <div class="event-parents">
                <i class="fas fa-arrow-up"></i>
                <span>Parents:</span>
                <div class="parent-tags">
                    ${parentTags}
                </div>
            </div>
        `;

    return card;
  }

  showEventModal(event) {
    const modal = document.getElementById("eventModal");
    const modalContent = document.getElementById("modalContent");

    const parentList =
      event.parents && event.parents.length > 0
        ? event.parents
            .map((parentId) => `<li>Event #${parentId}</li>`)
            .join("")
        : "<li><em>No parent events</em></li>";

    modalContent.innerHTML = `
            <div style="margin-bottom: 20px;">
                <strong>Event ID:</strong> ${event.id}
            </div>
            <div style="margin-bottom: 20px;">
                <strong>Data:</strong><br>
                <div style="background: #f3f4f6; padding: 15px; border-radius: 8px; margin-top: 8px;">
                    ${this.escapeHtml(event.data)}
                </div>
            </div>
            <div>
                <strong>Parent Events (${event.parent_count}):</strong>
                <ul style="margin-top: 8px; padding-left: 20px;">
                    ${parentList}
                </ul>
            </div>
        `;

    modal.style.display = "block";
  }

  updateStats() {
    const totalEvents = this.events.length;
    const totalRelationships = this.events.reduce(
      (sum, event) => sum + event.parent_count,
      0
    );

    document.getElementById("totalEvents").textContent = totalEvents;
    document.getElementById("totalRelationships").textContent =
      totalRelationships;
  }

  renderGraph() {
    const graphContainer = document.getElementById("graphContainer");

    if (this.events.length === 0) {
      graphContainer.innerHTML = `
                <div style="text-align: center;">
                    <i class="fas fa-project-diagram" style="font-size: 3rem; color: #d1d5db; margin-bottom: 15px;"></i>
                    <p>No events to visualize</p>
                </div>
            `;
      return;
    }

    // Create a simple text-based graph representation
    // In a real implementation, you'd use a library like D3.js or vis.js
    let graphHtml = '<div style="font-family: monospace; font-size: 0.9rem;">';

    this.events.forEach((event) => {
      graphHtml += `<div style="margin-bottom: 10px;">`;
      graphHtml += `<strong>${event.id}</strong>: ${this.escapeHtml(
        event.data.substring(0, 50)
      )}${event.data.length > 50 ? "..." : ""}`;

      if (event.parents && event.parents.length > 0) {
        graphHtml += `<br><span style="color: #6b7280; margin-left: 20px;">⬑ from: ${event.parents.join(
          ", "
        )}</span>`;
      }

      graphHtml += `</div>`;
    });

    graphHtml += "</div>";
    graphContainer.innerHTML = graphHtml;
  }

  exportData() {
    const dataStr = JSON.stringify(this.events, null, 2);
    const dataBlob = new Blob([dataStr], { type: "application/json" });
    const url = URL.createObjectURL(dataBlob);

    const link = document.createElement("a");
    link.href = url;
    link.download = `causaldb-export-${
      new Date().toISOString().split("T")[0]
    }.json`;
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
    URL.revokeObjectURL(url);

    this.showMessage("Data exported successfully!", "success");
  }

  showMessage(message, type) {
    // Remove existing messages
    const existingMessages = document.querySelectorAll(".message");
    existingMessages.forEach((msg) => msg.remove());

    const messageDiv = document.createElement("div");
    messageDiv.className = `message ${type}`;
    messageDiv.textContent = message;

    // Insert at the top of the content area
    const content = document.querySelector(".content");
    content.insertBefore(messageDiv, content.firstChild);

    // Auto-remove after 5 seconds
    setTimeout(() => {
      if (messageDiv.parentNode) {
        messageDiv.remove();
      }
    }, 5000);
  }

  escapeHtml(text) {
    const div = document.createElement("div");
    div.textContent = text;
    return div.innerHTML;
  }
}

// Initialize the frontend when the page loads
document.addEventListener("DOMContentLoaded", () => {
  new CausalDBFrontend();
});
