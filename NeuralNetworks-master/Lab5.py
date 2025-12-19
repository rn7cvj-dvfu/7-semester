import os
import time

import albumentations as A
import torch
import torch.nn.functional as F
from albumentations.pytorch import ToTensorV2
from PIL import Image
from sklearn.metrics import roc_auc_score
from torch.nn import (
    AdaptiveAvgPool2d,
    BatchNorm1d,
    BatchNorm2d,
    Conv2d,
    CrossEntropyLoss,
    Dropout,
    Linear,
    MaxPool2d,
    Module,
    Sequential,
)
from torch.nn.init import kaiming_normal_, zeros_
from torch.optim import Adam
from torch.optim.lr_scheduler import ReduceLROnPlateau
from torch.utils.data import DataLoader, Dataset
from torchvision import transforms
from tqdm import tqdm

IMG_FOLDERS = "horse-or-human"
TENSOR_FOLDER = "tensors/"

LABEL_CODE = {"horses": 0, "humans": 1}

BATCH_SIZE = 64
DEVICE = "cuda" if torch.cuda.is_available() else "cpu"


def save_images_as_tensors(path_from, path_to):
    subsets = {"train": "train", "validation": "val"}

    categories = ["horses", "humans"]

    resize_transform = transforms.Compose(
        [transforms.Resize((300, 300)), transforms.ToTensor()]
    )

    for subset_folder, prefix in subsets.items():
        for category in categories:
            folder_path = os.path.join(path_from, subset_folder, category)
            imgs = []

            for img_name in tqdm(os.listdir(folder_path)):
                if img_name.startswith("."):
                    continue
                img_path = os.path.join(folder_path, img_name)
                image = Image.open(img_path).convert("RGB")
                im = resize_transform(image)
                imgs.append(im)

            save_name = f"{prefix}_{category}.pt"
            torch.save(torch.stack(imgs), os.path.join(path_to, save_name))


def load_tensors(path_from):
    tensors = {}
    for tensor_name in tqdm(os.listdir(path_from)):
        path = os.path.join(path_from, tensor_name)
        tensors[tensor_name[:-3]] = torch.load(path)
    return tensors


class CustomDataset(Dataset):
    def __init__(self, tensors_dict, transform=None):
        self.images = []
        self.labels = []
        self.transform = transform

        for class_name, tensor_data in tensors_dict.items():
            label = LABEL_CODE[class_name]
            for idx in range(tensor_data.shape[0]):
                self.images.append(tensor_data[idx])
                self.labels.append(label)

    def __len__(self):
        return len(self.labels)

    def __getitem__(self, idx):
        image = self.images[idx]
        label = self.labels[idx]

        if self.transform:
            np_image = image.permute(1, 2, 0).numpy()
            transformed = self.transform(image=np_image)
            image = transformed["image"]

        return image, label


class ResidualBlock(Module):
    def __init__(self, in_channels, out_channels, stride=1):
        super().__init__()
        self.conv1 = Conv2d(
            in_channels,
            out_channels,
            kernel_size=3,
            stride=stride,
            padding=1,
            bias=False,
        )
        self.bn1 = BatchNorm2d(out_channels)
        self.conv2 = Conv2d(
            out_channels,
            out_channels,
            kernel_size=3,
            stride=1,
            padding=1,
            bias=False,
        )
        self.bn2 = BatchNorm2d(out_channels)

        self.shortcut = Sequential()
        if stride != 1 or in_channels != out_channels:
            self.shortcut = Sequential(
                Conv2d(
                    in_channels,
                    out_channels,
                    kernel_size=1,
                    stride=stride,
                    bias=False,
                ),
                BatchNorm2d(out_channels),
            )

    def forward(self, x):
        out = F.relu(self.bn1(self.conv1(x)))
        out = self.bn2(self.conv2(out))
        out += self.shortcut(x)
        out = F.relu(out)
        return out


class DeepResNet(Module):
    def _make_layer(self, block, out_channels, num_blocks, stride):
        strides = [stride] + [1] * (num_blocks - 1)
        layers = []
        for s in strides:
            layers.append(block(self.in_channels, out_channels, s))
            self.in_channels = out_channels
        return Sequential(*layers)

    def __init__(self):
        super().__init__()
        self.conv1 = Conv2d(3, 64, 7, 2, 3, bias=False)
        self.bn1 = BatchNorm2d(64)
        self.pool1 = MaxPool2d(3, 2, 1)

        self.in_channels = 64

        self.layer1 = self._make_layer(ResidualBlock, 64, 2, stride=1)
        self.layer2 = self._make_layer(ResidualBlock, 128, 2, stride=2)
        self.layer3 = self._make_layer(ResidualBlock, 256, 2, stride=2)
        self.layer4 = self._make_layer(ResidualBlock, 512, 2, stride=2)

        self.avg_pool = AdaptiveAvgPool2d((1, 1))

        self.fc = Sequential(
            Linear(512, 256, bias=False),
            BatchNorm1d(256),
            torch.nn.ReLU(),
            Dropout(0.5),
            Linear(256, 2),
        )
        self._init_weights()

    def _init_weights(self):
        for m in self.modules():
            if isinstance(m, Conv2d) or isinstance(m, Linear):
                kaiming_normal_(m.weight.data, nonlinearity="relu")
                if m.bias is not None:
                    zeros_(m.bias.data)

    def forward(self, x):
        x = self.pool1(F.relu(self.bn1(self.conv1(x))))
        x = self.layer1(x)
        x = self.layer2(x)
        x = self.layer3(x)
        x = self.layer4(x)
        x = self.avg_pool(x)
        x = torch.flatten(x, 1)
        x = self.fc(x)
        return x


def net_train(model, loader, optimizer, loss_fn):
    model.train()
    losses = 0
    cnt = 0
    correct = 0
    num_samples = 0

    for images, labels in loader:
        images = images.to(DEVICE)
        labels = labels.to(DEVICE)

        optimizer.zero_grad()
        outputs = model(images)
        loss = loss_fn(outputs, labels)
        loss.backward()
        optimizer.step()

        losses += loss.item()
        cnt += 1

        preds = torch.argmax(outputs, dim=1)
        correct += (preds == labels).sum().item()
        num_samples += labels.shape[0]

    return losses / cnt


def eval_net(model, loader, loss_fn):
    model.eval()
    losses = 0
    cnt = 0
    correct = 0
    num_samples = 0

    all_targets = []
    all_probs = []

    with torch.no_grad():
        for images, labels in loader:
            images = images.to(DEVICE)
            labels = labels.to(DEVICE)

            outputs = model(images)
            loss = loss_fn(outputs, labels)

            losses += loss.item()
            cnt += 1

            preds = torch.argmax(outputs, dim=1)
            correct += (preds == labels).sum().item()
            num_samples += labels.shape[0]

            probs = F.softmax(outputs, dim=1)[:, 1]
            all_targets.extend(labels.cpu().numpy())
            all_probs.extend(probs.cpu().numpy())

    roc_auc = roc_auc_score(all_targets, all_probs)

    return losses / cnt, roc_auc


def main():
    if not os.path.exists(
        os.path.join(os.getcwd(), TENSOR_FOLDER, "train_horses.pt")
    ):
        save_images_as_tensors(
            os.path.join(os.getcwd(), IMG_FOLDERS),
            os.path.join(os.getcwd(), TENSOR_FOLDER),
        )

    all_tensors = load_tensors(os.path.join(os.getcwd(), TENSOR_FOLDER))

    train_data = {
        "horses": all_tensors.get("train_horses"),
        "humans": all_tensors.get("train_humans"),
    }

    val_data = {
        "horses": all_tensors.get("val_horses"),
        "humans": all_tensors.get("val_humans"),
    }

    train_transforms = A.Compose(
        [
            A.Resize(300, 300),
            A.HorizontalFlip(p=0.5),
            A.Rotate(limit=20, p=0.5),
            A.GaussNoise(p=0.2),
            A.Normalize(mean=(0.485, 0.456, 0.406), std=(0.229, 0.224, 0.225)),
            ToTensorV2(),
        ]
    )

    val_transforms = A.Compose(
        [
            A.Resize(300, 300),
            A.Normalize(mean=(0.485, 0.456, 0.406), std=(0.229, 0.224, 0.225)),
            ToTensorV2(),
        ]
    )

    train_dataset = CustomDataset(train_data, transform=train_transforms)
    val_dataset = CustomDataset(val_data, transform=val_transforms)

    train_loader = DataLoader(train_dataset, BATCH_SIZE, shuffle=True)
    val_loader = DataLoader(val_dataset, BATCH_SIZE, shuffle=False)

    model = DeepResNet().to(DEVICE)
    optimizer = Adam(model.parameters(), lr=1e-4, weight_decay=1e-4)
    loss_fn = CrossEntropyLoss()
    scheduler = ReduceLROnPlateau(
        optimizer, mode="min", factor=0.1, patience=2
    )

    epochs = 10

    for epoch in range(epochs):
        s = time.time()

        train_loss = net_train(model, train_loader, optimizer, loss_fn)
        val_loss, val_auc = eval_net(model, val_loader, loss_fn)

        scheduler.step(val_loss)

        print(
            f"Epoch: {epoch + 1}/{epochs} | "
            f"Time: {int(time.time() - s)}s | "
            f"Train Loss: {train_loss:.4f} | "
            f"Val Loss: {val_loss:.4f} | "
            f"ROC-AUC: {val_auc:.4f}"
        )


if __name__ == "__main__":
    main()
